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

""" Module provides a base class for Tests """

import errno
import os
import subprocess
import shlex
import time
import sys
import traceback
import itertools
try:
    import simplejson as json
except ImportError:
    import json

from framework.core import Options
from framework.results import TestResult


__all__ = ['Test',
           'PiglitTest',
           'TEST_BIN_DIR']

# Platform global variables
if 'PIGLIT_PLATFORM' in os.environ:
    PIGLIT_PLATFORM = os.environ['PIGLIT_PLATFORM']
else:
    PIGLIT_PLATFORM = ''

if 'PIGLIT_BUILD_DIR' in os.environ:
    TEST_BIN_DIR = os.path.join(os.environ['PIGLIT_BUILD_DIR'], 'bin')
else:
    TEST_BIN_DIR = os.path.normpath(os.path.join(os.path.dirname(__file__),
                                                 '../bin'))


class Test(object):
    """ Abstract base class for Test classes

    This class provides the framework for running tests, with several methods
    and properties that can be overwritten to produce a specialized class for
    running test suites other than piglit.

    It provides two methods for running tests, excecute and run.
    execute() provides lots of features, and is invoced when running piglit
    from the command line, run() is a more basic method for running the test,
    and is called internally by execute(), but is can be useful outside of it.

    Arguments:
    command -- a value to be passed to subprocess.Popen

    Keyword Arguments:
    run_concurrent -- If True the test is thread safe. Default: False

    """
    OPTS = Options()
    __slots__ = ['run_concurrent', 'env', 'result', 'cwd', '_command',
                 '_test_hook_execute_run']

    def __init__(self, command, run_concurrent=False):
        self._command = None
        self.run_concurrent = run_concurrent
        self.command = command
        self.env = {}
        self.result = TestResult({'result': 'fail'})
        self.cwd = None

        # This is a hook for doing some testing on execute right before
        # self.run is called.
        self._test_hook_execute_run = lambda: None

    def execute(self, path, log, json_writer, dmesg):
        """ Run a test

        Run a test, but with features. This times the test, uses dmesg checking
        (if requested), and runs the logger.

        Arguments:
        path -- the name of the test
        log -- a log.Log instance
        json_writer -- a results.JSONWriter instance
        dmesg -- a dmesg.BaseDmesg derived class

        """
        log_current = log.pre_log(path if self.OPTS.verbose else None)

        # Run the test
        if self.OPTS.execute:
            try:
                time_start = time.time()
                dmesg.update_dmesg()
                self._test_hook_execute_run()
                self.run()
                self.result['time'] = time.time() - time_start
                self.result = dmesg.update_result(self.result)
            # This is a rare case where a bare exception is okay, since we're
            # using it to log exceptions
            except:
                exception = sys.exc_info()
                self.result['result'] = 'fail'
                self.result['exception'] = "{}{}".format(*exception[:2])
                self.result['traceback'] = "".join(
                    traceback.format_tb(exception[2]))

            log.log(path, self.result['result'])
            log.post_log(log_current, self.result['result'])

            json_writer.write_dict_item(path, self.result)
        else:
            log.log(path, 'dry-run')
            log.post_log(log_current, 'dry-run')

    @property
    def command(self):
        assert self._command
        if self.OPTS.valgrind:
            return ['valgrind', '--quiet', '--error-exitcode=1',
                    '--tool=memcheck'] + self._command
        return self._command

    @command.setter
    def command(self, value):
        if isinstance(value, basestring):
            self._command = shlex.split(str(value))
            return
        self._command = value

    def interpret_result(self):
        raise NotImplementedError

    def run(self):
        """
        Run a test.  The return value will be a dictionary with keys
        including 'result', 'info', 'returncode' and 'command'.
        * For 'result', the value may be one of 'pass', 'fail', 'skip',
          'crash', or 'warn'.
        * For 'info', the value will include stderr/out text.
        * For 'returncode', the value will be the numeric exit code/value.
        * For 'command', the value will be command line program and arguments.
        """
        self.result['command'] = ' '.join(self.command)
        self.result['environment'] = " ".join(
            '{0}="{1}"'.format(k, v) for k, v in itertools.chain(
                self.OPTS.env.iteritems(), self.env.iteritems()))

        if self.check_for_skip_scenario():
            self.result['result'] = 'skip'
            self.result['out'] = "skipped by self.is_skip()"
            self.result['err'] = ""
            self.result['returncode'] = None
            return

        # https://bugzilla.gnome.org/show_bug.cgi?id=680214 is affecting many
        # developers. If we catch it happening, try just re-running the test.
        for _ in xrange(5):
            self.__run_command()
            if "Got spurious window resize" not in self.result['out']:
                break

        # If the result is skip then the test wasn't run, return early
        # This usually is triggered when a test is not built for a specific
        # platform
        if self.result['result'] == 'skip':
            return

        self.result['result'] = 'fail'
        self.interpret_result()

        if self.result['returncode'] < 0:
            self.result['result'] = 'crash'
        elif self.result['returncode'] != 0 and self.result['result'] == 'pass':
            self.result['result'] = 'warn'

        if self.OPTS.valgrind:
            # If the underlying test failed, simply report
            # 'skip' for this valgrind test.
            if self.result['result'] != 'pass':
                self.result['result'] = 'skip'
            elif self.result['returncode'] == 0:
                # Test passes and is valgrind clean.
                self.result['result'] = 'pass'
            else:
                # Test passed but has valgrind errors.
                self.result['result'] = 'fail'

    def check_for_skip_scenario(self):
        """ Application specific check for skip

        If this function returns a truthy value then the current test will be
        skipped. The base version will always return False

        """
        return False

    def __run_command(self):
        """ Run the test command and get the result

        This method sets environment options, then runs the executable. If the
        executable isn't found it sets the result to skip.

        """
        # Set the environment for the tests. Use the default settings created
        # in the Options constructor first, then use any user defined
        # variables, finally, use any variables set for the test in the test
        # profile
        fullenv = self.OPTS.env.copy()
        for key, value in itertools.chain(self.env.iteritems(),
                                          os.environ.iteritems()):
            fullenv[key] = str(value)

        try:
            proc = subprocess.Popen(self.command,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    cwd=self.cwd,
                                    env=fullenv,
                                    universal_newlines=True)
            out, err = proc.communicate()
            returncode = proc.returncode
        except OSError as e:
            # Different sets of tests get built under
            # different build configurations.  If
            # a developer chooses to not build a test,
            # Piglit should not report that test as having
            # failed.
            if e.errno == errno.ENOENT:
                self.result['result'] = 'skip'
                out = "Test executable not found.\n"
                err = ""
                returncode = None
            else:
                raise e

        # proc.communicate() returns 8-bit strings, but we need
        # unicode strings.  In Python 2.x, this is because we
        # will eventually be serializing the strings as JSON,
        # and the JSON library expects unicode.  In Python 3.x,
        # this is because all string operations require
        # unicode.  So translate the strings into unicode,
        # assuming they are using UTF-8 encoding.
        #
        # If the subprocess output wasn't properly UTF-8
        # encoded, we don't want to raise an exception, so
        # translate the strings using 'replace' mode, which
        # replaces erroneous charcters with the Unicode
        # "replacement character" (a white question mark inside
        # a black diamond).
        self.result['out'] = out.decode('utf-8', 'replace')
        self.result['err'] = err.decode('utf-8', 'replace')
        self.result['returncode'] = returncode


class PiglitTest(Test):
    """
    PiglitTest: Run a "native" piglit test executable

    Expect one line prefixed PIGLIT: in the output, which contains a result
    dictionary. The plain output is appended to this dictionary
    """
    def __init__(self, *args, **kwargs):
        super(PiglitTest, self).__init__(*args, **kwargs)

        # Prepend TEST_BIN_DIR to the path.
        self._command[0] = os.path.join(TEST_BIN_DIR, self._command[0])

    def check_for_skip_scenario(self):
        """ Native Piglit-test specific skip checking

        If we are running on gbm don't run glean or glx- tests

        """
        if PIGLIT_PLATFORM == 'gbm':
            split_command = os.path.split(self._command[0])[1]
            if 'glean' == split_command:
                return True
            if split_command.startswith('glx-'):
                return True
        return False

    def interpret_result(self):
        outlines = self.result['out'].split('\n')
        outpiglit = (s[7:] for s in outlines if s.startswith('PIGLIT:'))

        for piglit in outpiglit:
            self.result.recursive_update(json.loads(piglit))
        self.result['out'] = '\n'.join(
            s for s in outlines if not s.startswith('PIGLIT:'))
