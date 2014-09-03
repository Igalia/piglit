# Copyright (c) 2014 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

""" Module providing json backend for piglit """

import os
import threading
try:
    import simplejson as json
except ImportError:
    import json
import framework.status as status
from .abstract import Backend, FSyncMixin

__all__ = [
    'CURRENT_JSON_VERSION',
    'JSONBackend',
    'piglit_encoder',
]


# The current version of the JSON results
CURRENT_JSON_VERSION = 1


def piglit_encoder(obj):
    """ Encoder for piglit that can transform additional classes into json

    Adds support for status.Status objects and for set() instances

    """
    if isinstance(obj, status.Status):
        return str(obj)
    elif isinstance(obj, set):
        return list(obj)
    return obj


class JSONBackend(FSyncMixin, Backend):
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
        w._open_dict()
        w._write_dict_item('a', [1, 2, 3])
        w._write_dict_item('b', 4)
        w._write_dict_item('c', {'x': 100})
        w._close_dict()

    which is also equivalent to::
        w = JSONWriter(file)
        w._open_dict()
        w._write_dict_item('a', [1, 2, 3])
        w._write_dict_item('b', 4)

        w._write_dict_key('c')
        w._open_dict()
        w._write_dict_item('x', 100)
        w._close_dict()

        w._close_dict()
    '''

    INDENT = 4
    _LOCK = threading.RLock()

    def __init__(self, f, metadata, **options):
        self._file = open(os.path.join(f, 'results.json'), 'w')
        FSyncMixin.__init__(self, **options)
        self.__indent_level = 0
        self.__inhibit_next_indent = False
        self.__encoder = json.JSONEncoder(indent=self.INDENT,
                                          default=piglit_encoder)

        # self.__is_collection_empty
        #
        # A stack that indicates if the currect collection is empty
        #
        # When _open_dict is called, True is pushed onto the
        # stack. When the first element is written to the newly
        # opened dict, the top of the stack is set to False.
        # When the _close_dict is called, the stack is popped.
        #
        # The top of the stack is element -1.
        #
        self.__is_collection_empty = []

        # self._open_containers
        #
        # A FILO stack that stores container information, each time
        # self._open_dict() 'dict' is added to the stack, (other elements like
        # 'list' could be added if support was added to JSONWriter for handling
        # them), each to time self._close_dict() is called an element is
        # removed. When self.close_json() is called each element of the stack
        # is popped and written into the json
        self._open_containers = []

        # Write initial metadata into the backend store
        self._initialize(metadata)

    def _initialize(self, metadata):
        """ Write boilerplate json code

        This writes all of the json except the actual tests.

        Arguments:
        options -- any values to be put in the options dictionary, must be a
                   dict-like object
        name -- the name of the test
        env -- any environment information to be written into the results, must
               be a dict-like object

        """
        with self._LOCK:
            self._open_dict()
            self._write_dict_item('results_version', CURRENT_JSON_VERSION)
            self._write_dict_item('name', metadata['name'])

            self._write_dict_key('options')
            self._open_dict()
            for key, value in metadata.iteritems():
                # Dont' write env or name into the options dictionary
                if key in ['env', 'name']:
                    continue

                # Loading a NoneType will break resume, and are a bug
                assert value is not None, "Value {} is NoneType".format(key)
                self._write_dict_item(key, value)
            self._close_dict()

            for key, value in metadata['env'].iteritems():
                self._write_dict_item(key, value)

            # Open the tests dictinoary so that tests can be written
            self._write_dict_key('tests')
            self._open_dict()

    def finalize(self, metadata=None):
        """ End json serialization and cleanup

        This method is called after all of tests are written, it closes any
        containers that are still open and closes the file

        """
        # Ensure that there are no tests still writing by taking the lock here
        with self._LOCK:
            # Close the tests dictionary
            self._close_dict()

            # Write closing metadata
            if metadata:
                for key, value in metadata.iteritems():
                    self._write_dict_item(key, value)

            # Close the root dictionary object
            self._close_dict()

            # Close the file.
            assert self._open_containers == [], \
                "containers stack: {0}".format(self._open_containers)
            self._file.close()

    def __write_indent(self):
        if self.__inhibit_next_indent:
            self.__inhibit_next_indent = False
            return
        else:
            i = ' ' * self.__indent_level * self.INDENT
            self._file.write(i)

    def __write(self, obj):
        lines = list(self.__encoder.encode(obj).split('\n'))
        n = len(lines)
        for i in range(n):
            self.__write_indent()
            self._file.write(lines[i])
            if i != n - 1:
                self._file.write('\n')

    def _open_dict(self):
        self.__write_indent()
        self._file.write('{')

        self.__indent_level += 1
        self.__is_collection_empty.append(True)
        self._open_containers.append('dict')
        self._fsync(self._file)

    def _close_dict(self):
        self.__indent_level -= 1
        self.__is_collection_empty.pop()

        self._file.write('\n')
        self.__write_indent()
        self._file.write('}')
        assert self._open_containers[-1] == 'dict'
        self._open_containers.pop()
        self._fsync(self._file)

    def _write_dict_item(self, key, value):
        # Write key.
        self._write_dict_key(key)

        # Write value.
        self.__write(value)

        self._fsync(self._file)

    def _write_dict_key(self, key):
        # Write comma if this is not the initial item in the dict.
        if self.__is_collection_empty[-1]:
            self.__is_collection_empty[-1] = False
        else:
            self._file.write(',')

        self._file.write('\n')
        self.__write(key)
        self._file.write(': ')

        self.__inhibit_next_indent = True
        self._fsync(self._file)

    def write_test(self, name, data):
        """ Write a test into the JSON tests dictionary """
        with self._LOCK:
            self._write_dict_item(name, data)
