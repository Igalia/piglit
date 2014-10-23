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
import shutil
try:
    import simplejson as json
except ImportError:
    import json
import framework.status as status
from .abstract import FileBackend

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


class JSONBackend(FileBackend):
    """ Piglit's native JSON backend

    This writes out to piglit's native json backend. This class uses the python
    json module or the simplejson.

    This class is atomic, writes either completely fail or completley succeed.
    To achieve this it writes individual files for each test and for the
    metadata, and composes them at the end into a single file and removes the
    intermediate files. When it tries to compose these files if it cannot read
    a file it just ignores it, making the result atomic.

    """
    INDENT = 4

    def initialize(self, metadata):
        """ Write boilerplate json code

        This writes all of the json except the actual tests.

        Arguments:
        metadata -- a dictionary of values to be written

        """
        # If metadata is None then this is a loaded result and there is no need
        # to initialize
        metadata['results_version'] = CURRENT_JSON_VERSION

        with open(os.path.join(self._dest, 'metadata.json'), 'w') as f:
            json.dump(metadata, f, default=piglit_encoder)

        # make the directory for the tests
        try:
            os.mkdir(os.path.join(self._dest, 'tests'))
        except OSError:
            pass

    def finalize(self, metadata=None):
        """ End json serialization and cleanup

        This method is called after all of tests are written, it closes any
        containers that are still open and closes the file

        """
        # Create a dictionary that is full of data to be written to a single
        # file
        data = {}

        # Load the metadata and put it into a dictionary
        with open(os.path.join(self._dest, 'metadata.json'), 'r') as f:
            data.update(json.load(f))

        # If there is more metadata add it the dictionary
        if metadata:
            data.update(metadata)

        # Add the tests to the dictionary
        data['tests'] = {}

        test_dir = os.path.join(self._dest, 'tests')
        for test in os.listdir(test_dir):
            test = os.path.join(test_dir, test)
            if os.path.isfile(test):
                # Try to open the json snippets. If we fail to open a test then
                # throw the whole thing out. This gives us atomic writes, the
                # writing worked and is valid or it didn't work.
                try:
                    with open(test, 'r') as f:
                        data['tests'].update(json.load(f))
                except ValueError:
                    pass
        assert data['tests']

        # write out the combined file.
        with open(os.path.join(self._dest, 'results.json'), 'w') as f:
            json.dump(data, f, default=piglit_encoder,
                      indent=JSONBackend.INDENT)

        # Delete the temporary files
        os.unlink(os.path.join(self._dest, 'metadata.json'))
        shutil.rmtree(os.path.join(self._dest, 'tests'))

    def write_test(self, name, data):
        """ Write a test into the JSON tests dictionary """
        t = os.path.join(self._dest, 'tests',
                         '{}.json'.format(self._counter.next()))
        with open(t, 'w') as f:
            json.dump({name: data}, f, default=piglit_encoder)
            self._fsync(f)
