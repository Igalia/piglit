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

""" Module for results generation """

from __future__ import print_function
import os
from cStringIO import StringIO
try:
    import simplejson as json
except ImportError:
    import json

import framework.status as status
from framework.threads import synchronized_self

__all__ = [
    'TestrunResult',
    'TestResult',
    'JSONWriter',
    'load_results',
]


def _piglit_encoder(obj):
    """ Encoder for piglit that can transform additional classes into json

    Adds support for status.Status objects and for set() instances

    """
    if isinstance(obj, status.Status):
        return str(obj)
    elif isinstance(obj, set):
        return list(obj)
    return obj


class JSONWriter(object):
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
        self.__encoder = json.JSONEncoder(indent=self.INDENT,
                                          default=_piglit_encoder)

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


class TestResult(dict):
    def __init__(self, *args):
        super(TestResult, self).__init__(*args)

        # Replace the result with a status object
        try:
            self['result'] = status.status_lookup(self['result'])
        except KeyError:
            # If there isn't a result (like when used by piglit-run), go on
            # normally
            pass


class TestrunResult(object):
    def __init__(self, resultfile=None):
        self.serialized_keys = ['options',
                                'name',
                                'tests',
                                'uname',
                                'wglinfo',
                                'glxinfo',
                                'lspci',
                                'time_elapsed']
        self.name = None
        self.uname = None
        self.options = None
        self.glxinfo = None
        self.lspci = None
        self.time_elapsed = None
        self.tests = {}

        if resultfile:
            # Attempt to open the json file normally, if it fails then attempt
            # to repair it.
            try:
                raw_dict = json.load(resultfile)
            except ValueError:
                raw_dict = json.load(self.__repairFile(resultfile))

            # Check that only expected keys were unserialized.
            for key in raw_dict:
                if key not in self.serialized_keys:
                    raise Exception('unexpected key in results file: ', str(key))

            self.__dict__.update(raw_dict)

            # Replace each raw dict in self.tests with a TestResult.
            for (path, result) in self.tests.items():
                self.tests[path] = TestResult(result)

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
        safe_line = 2 * JSONWriter.INDENT * ' ' + '},\n'

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
        lines[-1] = 2 * JSONWriter.INDENT * ' ' + '}\n'

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


def load_results(filename):
    """ Loader function for TestrunResult class

    This function takes a single argument of a results file.

    It makes quite a few assumptions, first it assumes that it has been passed
    a folder, if that fails then it looks for a plain text json file called
    "main"

    """
    try:
        with open(filename, 'r') as resultsfile:
            testrun = TestrunResult(resultsfile)
    except IOError:
        with open(os.path.join(filename, "main"), 'r') as resultsfile:
            testrun = TestrunResult(resultsfile)

    assert(testrun.name is not None)
    return testrun
