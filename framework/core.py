#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

# Piglit core

import errno
import json
import os
import platform
import re
import stat
import subprocess
import string
import sys
import time
import traceback
from log import log
from cStringIO import StringIO
from textwrap import dedent
from threads import ConcurrentTestPool
from threads import synchronized_self
import threading

__all__ = ['Environment',
           'checkDir',
           'loadTestProfile',
           'TestrunResult',
           'GroupResult',
           'TestResult',
           'TestProfile',
           'Group',
           'Test',
           'testBinDir']


class JSONWriter:
    '''
    Writes to a JSON file stream

    JSONWriter is threadsafe.

    Example
    -------

    This call to ``json.dump``::
        json.dump(
            {
                'a': [1, 2, 3],
                'b': 4,
                'c': {
                    'x': 100,
                },
            }
            file,
            indent=JSONWriter.INDENT)

    is equivalent to::
        w = JSONWriter(file)
        w.open_dict()
        w.write_dict_item('a', [1, 2, 3])
        w.write_dict_item('b', 4)
        w.write_dict_item('c', {'x': 100})
        w.close_dict()

    which is also equivalent to::
        w = JSONWriter(file)
        w.open_dict()
        w.write_dict_item('a', [1, 2, 3])
        w.write_dict_item('b', 4)

        w.write_dict_key('c')
        w.open_dict()
        w.write_dict_item('x', 100)
        w.close_dict()

        w.close_dict()
    '''

    INDENT = 4

    def __init__(self, file):
        self.file = file
        self.__indent_level = 0
        self.__inhibit_next_indent = False
        self.__encoder = json.JSONEncoder(indent=self.INDENT)

        # self.__is_collection_empty
        #
        # A stack that indicates if the currect collection is empty
        #
        # When open_dict is called, True is pushed onto the
        # stack. When the first element is written to the newly
        # opened dict, the top of the stack is set to False.
        # When the close_dict is called, the stack is popped.
        #
        # The top of the stack is element -1.
        #
        # XXX: How does one attach docstrings to member variables?
        #
        self.__is_collection_empty = []

    @synchronized_self
    def __write_indent(self):
        if self.__inhibit_next_indent:
            self.__inhibit_next_indent = False
            return
        else:
            i = ' ' * self.__indent_level * self.INDENT
            self.file.write(i)

    @synchronized_self
    def __write(self, obj):
        lines = list(self.__encoder.encode(obj).split('\n'))
        n = len(lines)
        for i in range(n):
            self.__write_indent()
            self.file.write(lines[i])
            if i != n - 1:
                self.file.write('\n')

    @synchronized_self
    def open_dict(self):
        self.__write_indent()
        self.file.write('{')

        self.__indent_level += 1
        self.__is_collection_empty.append(True)

    @synchronized_self
    def close_dict(self, comma=True):
        self.__indent_level -= 1
        self.__is_collection_empty.pop()

        self.file.write('\n')
        self.__write_indent()
        self.file.write('}')

    @synchronized_self
    def write_dict_item(self, key, value):
        # Write key.
        self.write_dict_key(key)

        # Write value.
        self.__write(value)

    @synchronized_self
    def write_dict_key(self, key):
        # Write comma if this is not the initial item in the dict.
        if self.__is_collection_empty[-1]:
            self.__is_collection_empty[-1] = False
        else:
            self.file.write(',')

        self.file.write('\n')
        self.__write(key)
        self.file.write(': ')

        self.__inhibit_next_indent = True


# Ensure the given directory exists
def checkDir(dirname, failifexists):
    exists = True
    try:
        os.stat(dirname)
    except OSError as e:
        if e.errno == errno.ENOENT or e.errno == errno.ENOTDIR:
            exists = False

    if exists and failifexists:
        print >>sys.stderr, "%(dirname)s exists already.\nUse --overwrite if" \
                            "you want to overwrite it.\n" % locals()
        exit(1)

    try:
        os.makedirs(dirname)
    except OSError as e:
        if e.errno != errno.EEXIST:
            raise

if 'PIGLIT_BUILD_DIR' in os.environ:
    testBinDir = os.environ['PIGLIT_BUILD_DIR'] + '/bin/'
else:
    testBinDir = os.path.dirname(__file__) + '/../bin/'

if 'PIGLIT_SOURCE_DIR' not in os.environ:
    p = os.path
    os.environ['PIGLIT_SOURCE_DIR'] = p.abspath(p.join(p.dirname(__file__),
                                                       '..'))


class TestResult(dict):
    pass


class GroupResult(dict):
    def get_subgroup(self, path, create=True):
        '''
        Retrieve subgroup specified by path

        For example, ``self.get_subgroup('a/b/c')`` will attempt to
        return ``self['a']['b']['c']``. If any subgroup along ``path``
        does not exist, then it will be created if ``create`` is true;
        otherwise, ``None`` is returned.
        '''
        group = self
        for subname in path.split('/'):
            if subname not in group:
                if create:
                    group[subname] = GroupResult()
                else:
                    return None
            group = group[subname]
            assert(isinstance(group, GroupResult))
        return group

    @staticmethod
    def make_tree(tests):
        '''
        Convert a flat dict of test results to a hierarchical tree

        ``tests`` is a dict whose items have form ``(path, TestResult)``,
        where path is a string with form ``group1/group2/.../test_name``.

        Return a tree whose leaves are the values of ``tests`` and
        whose nodes, which have type ``GroupResult``, reflect the
        paths in ``tests``.
        '''
        root = GroupResult()

        for (path, result) in tests.items():
            group_path = os.path.dirname(path)
            test_name = os.path.basename(path)

            group = root.get_subgroup(group_path)
            group[test_name] = TestResult(result)

        return root


class TestrunResult:
    def __init__(self):
        self.serialized_keys = ['options',
                                'name',
                                'tests',
                                'wglinfo',
                                'glxinfo',
                                'lspci',
                                'time_elapsed']
        self.name = None
        self.glxinfo = None
        self.lspci = None
        self.time_elapsed = None
        self.tests = {}

    def __repairFile(self, file):
        '''
        Reapair JSON file if necessary

        If the JSON file is not closed properly, perhaps due a system
        crash during a test run, then the JSON is repaired by
        discarding the trailing, incomplete item and appending braces
        to the file to close the JSON object.

        The repair is performed on a string buffer, and the given file
        is never written to. This allows the file to be safely read
        during a test run.

        :return: If no repair occured, then ``file`` is returned.
                Otherwise, a new file object containing the repaired JSON
                is returned.
        '''

        file.seek(0)
        lines = file.readlines()

        # JSON object was not closed properly.
        #
        # To repair the file, we execute these steps:
        #   1. Find the closing brace of the last, properly written
        #      test result.
        #   2. Discard all subsequent lines.
        #   3. Remove the trailing comma of that test result.
        #   4. Append enough closing braces to close the json object.
        #   5. Return a file object containing the repaired JSON.

        # Each non-terminal test result ends with this line:
        safe_line = 3 * JSONWriter.INDENT * ' ' + '},\n'

        # Search for the last occurence of safe_line.
        safe_line_num = None
        for i in range(-1, - len(lines), -1):
            if lines[i] == safe_line:
                safe_line_num = i
                break

        if safe_line_num is None:
            raise Exception('failed to repair corrupt result file: ' +
                            file.name)

        # Remove corrupt lines.
        lines = lines[0:(safe_line_num + 1)]

        # Remove trailing comma.
        lines[-1] = 3 * JSONWriter.INDENT * ' ' + '}\n'

        # Close json object.
        lines.append(JSONWriter.INDENT * ' ' + '}\n')
        lines.append('}')

        # Return new file object containing the repaired JSON.
        new_file = StringIO()
        new_file.writelines(lines)
        new_file.flush()
        new_file.seek(0)
        return new_file

    def write(self, file):
        # Serialize only the keys in serialized_keys.
        keys = set(self.__dict__.keys()).intersection(self.serialized_keys)
        raw_dict = dict([(k, self.__dict__[k]) for k in keys])
        json.dump(raw_dict, file, indent=JSONWriter.INDENT)

    def parseFile(self, file):
        # Attempt to open the json file normally, if it fails then attempt to
        # repair it.
        try:
            raw_dict = json.load(file)
        except ValueError:
            raw_dict = json.load(self.__repairFile(file))

        # Check that only expected keys were unserialized.
        for key in raw_dict:
            if key not in self.serialized_keys:
                raise Exception('unexpected key in results file: ', str(key))

        self.__dict__.update(raw_dict)

        # Replace each raw dict in self.tests with a TestResult.
        for (path, result) in self.tests.items():
            self.tests[path] = TestResult(result)


class Environment:
    def __init__(self, concurrent=True, execute=True, include_filter=[],
                 exclude_filter=[], valgrind=False):
        self.concurrent = concurrent
        self.execute = execute
        self.filter = []
        self.exclude_filter = []
        self.exclude_tests = set()
        self.valgrind = valgrind

        """
        The filter lists that are read in should be a list of string objects,
        however, the filters need to be a list or regex object.

        This code uses re.compile to rebuild the lists and set self.filter
        """
        for each in include_filter:
            self.filter.append(re.compile(each))
        for each in exclude_filter:
            self.exclude_filter.append(re.compile(each))

    def run(self, command):
        try:
            p = subprocess.Popen(command,
                                 stdout=subprocess.PIPE,
                                 stderr=subprocess.PIPE,
                                 universal_newlines=True)
            (stdout, stderr) = p.communicate()
        except:
            return "Failed to run " + command
        return stderr+stdout

    def collectData(self):
        result = {}
        system = platform.system()
        if (system == 'Windows' or system.find("CYGWIN_NT") == 0):
            result['wglinfo'] = self.run('wglinfo')
        else:
            result['glxinfo'] = self.run('glxinfo')
        if system == 'Linux':
            result['lspci'] = self.run('lspci')
        return result


class Test:
    ignoreErrors = []

    def __init__(self, runConcurrent=False):
        '''
                'runConcurrent' controls whether this test will
                execute it's work (i.e. __doRunWork) on the calling thread
                (i.e. the main thread) or from the ConcurrentTestPool threads.
        '''
        self.runConcurrent = runConcurrent
        self.skip_test = False

    def run(self):
        raise NotImplementedError

    def schedule(self, env, path, json_writer):
        '''
        Schedule test to be run via the concurrent thread pool.
        This is a no-op if the test isn't marked as concurrent.

        See ``Test.doRun`` for a description of the parameters.
        '''
        args = (env, path, json_writer)
        if self.runConcurrent:
            ConcurrentTestPool().put(self.doRun, args=args)

    def doRun(self, env, path, json_writer):
        '''
        Run the test immediately.

        :path:
            Fully qualified test name as a string.  For example,
            ``spec/glsl-1.30/preprocessor/compiler/keywords/void.frag``.
        '''
        def status(msg):
            log(msg=msg, channel=path)

        # Run the test
        if env.execute:
            try:
                status("running")
                time_start = time.time()
                result = self.run(env.valgrind)
                time_end = time.time()
                if 'time' not in result:
                    result['time'] = time_end - time_start
                if 'result' not in result:
                    result['result'] = 'fail'
                if not isinstance(result, TestResult):
                    result = TestResult(result)
                    result['result'] = 'warn'
                    result['note'] = 'Result not returned as an instance ' \
                                     'of TestResult'
            except:
                result = TestResult()
                result['result'] = 'fail'
                result['exception'] = str(sys.exc_info()[0]) + \
                    str(sys.exc_info()[1])
                result['traceback'] = \
                    "".join(traceback.format_tb(sys.exc_info()[2]))

            status(result['result'])

            if 'subtest' in result and len(result['subtest'].keys()) > 1:
                for test in result['subtest'].keys():
                    result['result'] = result['subtest'][test]
                    json_writer.write_dict_item(path + '/' + test, result)
            else:
                json_writer.write_dict_item(path, result)
        else:
            status("dry-run")

    # Returns True iff the given error message should be ignored
    def isIgnored(self, error):
        for pattern in Test.ignoreErrors:
            if pattern.search(error):
                return True

        return False

    # Default handling for stderr messages
    def handleErr(self, results, err):
        errors = filter(lambda s: len(s) > 0,
                        map(lambda s: s.strip(), err.split('\n')))

        ignored = [s for s in errors if self.isIgnored(s)]
        errors = [s for s in errors if s not in ignored]

        if len(errors) > 0:
            results['errors'] = errors

            if results['result'] == 'pass':
                results['result'] = 'warn'

        if len(ignored) > 0:
            results['errors_ignored'] = ignored


class Group(dict):
    pass


class TestProfile:
    def __init__(self):
        self.tests = Group()
        self.test_list = {}

    def flatten_group_hierarchy(self):
        '''
        Convert Piglit's old hierarchical Group() structure into a flat
        dictionary mapping from fully qualified test names to "Test" objects.

        For example,
        tests['spec']['glsl-1.30']['preprocessor']['compiler']['void.frag']
        would become:
        test_list['spec/glsl-1.30/preprocessor/compiler/void.frag']
        '''

        def f(prefix, group, test_dict):
            for key in group:
                fullkey = key if prefix == '' else prefix + '/' + key
                if isinstance(group[key], dict):
                    f(fullkey, group[key], test_dict)
                else:
                    test_dict[fullkey] = group[key]
        f('', self.tests, self.test_list)
        # Clear out the old Group()
        self.tests = Group()

    def prepare_test_list(self, env):
        self.flatten_group_hierarchy()

        def matches_any_regexp(x, re_list):
            return True in map(lambda r: r.search(x) is not None, re_list)

        def test_matches(item):
            path, test = item
            return ((not env.filter or matches_any_regexp(path, env.filter))
                    and not path in env.exclude_tests and
                    not matches_any_regexp(path, env.exclude_filter))

        # Filter out unwanted tests
        self.test_list = dict(filter(test_matches, self.test_list.items()))

    def run(self, env, json_writer):
        '''
        Schedule all tests in profile for execution.

        See ``Test.schedule`` and ``Test.run``.
        '''

        self.prepare_test_list(env)

        # Queue up all the concurrent tests, so the pool is filled
        # at the start of the test run.
        if env.concurrent:
            for (path, test) in self.test_list.items():
                test.schedule(env, path, json_writer)

        # Run any remaining non-concurrent tests serially from this
        # thread, while the concurrent tests
        for (path, test) in self.test_list.items():
            if not env.concurrent or not test.runConcurrent:
                test.doRun(env, path, json_writer)
        ConcurrentTestPool().join()

    def remove_test(self, test_path):
        """Remove a fully qualified test from the profile.

        ``test_path`` is a string with slash ('/') separated
        components. It has no leading slash. For example::
                test_path = 'spec/glsl-1.30/linker/do-stuff'
        """

        l = test_path.split('/')
        group = self.tests[l[0]]
        for group_name in l[1:-2]:
            group = group[group_name]
        del group[l[-1]]


def loadTestProfile(filename):
    ns = {'__file__': filename}
    try:
        execfile(filename, ns)
    except:
        traceback.print_exc()
        raise Exception('Could not read tests profile')
    return ns['profile']


def loadTestResults(relativepath):
    path = os.path.realpath(relativepath)
    if os.path.isdir(path):
        filepath = os.path.join(path, 'main')
    else:
        filepath = path

    testrun = TestrunResult()
    try:
        with open(filepath, 'r') as file:
            testrun.parseFile(file)
    except OSError:
        traceback.print_exc()
        raise Exception('Could not read tests results')

    assert(testrun.name is not None)
    return testrun

# Error messages to be ignored
Test.ignoreErrors = map(re.compile,
                        ["couldn't open libtxc_dxtn.so",
                         "compression/decompression available",
                         "Mesa: .*build",
                         "Mesa: CPU.*",
                         "Mesa: .*cpu detected.",
                         "Mesa: Test.*",
                         "Mesa: Yes.*",
                         "libGL: XF86DRIGetClientDriverName.*",
                         "libGL: OpenDriver: trying.*",
                         "libGL: Warning in.*drirc*",
                         "ATTENTION.*value of option.*",
                         "drmOpen.*",
                         "Mesa: Not testing OS support.*",
                         "Mesa: User error:.*",
                         "Mesa: Initializing .* optimizations",
                         "debug_get_.*",
                         "util_cpu_caps.*",
                         "Mesa: 3Dnow! detected",
                         "r300:.*",
                         "radeon:.*",
                         "Warning:.*",
                         "0 errors, .*",
                         "Mesa.*",
                         "no rrb",
                         "; ModuleID.*",
                         "%.*",
                         ".*failed to translate tgsi opcode.*to SSE",
                         ".*falling back to interpreter",
                         "GLSL version is .*, but requested version .* is "
                         "required",
                         "kCGErrorIllegalArgument: CGSOrderWindowList",
                         "kCGErrorFailure: Set a breakpoint @ "
                         "CGErrorBreakpoint\(\) to catch errors as they are "
                         "logged.",
                         "stw_(init|cleanup).*",
                         "OpenGLInfo..*",
                         "AdapterInfo..*",
                         "frameThrottleRate.*",
                         ".*DeviceName.*",
                         "No memory leaks detected.",
                         "libGL: Can't open configuration file.*"])


def parse_listfile(filename):
    """
    Parses a newline-seperated list in a text file and returns a python list
    object. It will expand tildes on Unix-like system to the users home
    directory.

    ex file.txt:
        ~/tests1
        ~/tests2/main
        /tmp/test3

    returns:
        ['/home/user/tests1', '/home/users/tests2/main', '/tmp/test3']
    """
    with open(filename, 'r') as file:
        return [path.expanduser(i.rstrip('\n')) for i in file.readlines()]
