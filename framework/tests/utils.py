# Copyright (c) 2014 Intel Coporation

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

""" Common helpers for framework tests

This module collects common tools that are needed in more than one test module
in a single place.

"""

import os
import shutil
import tempfile
from contextlib import contextmanager
try:
    import simplejson as json
except ImportError:
    import json


__all__ = [
    'with_tempfile',
    'resultfile',
    'tempdir',
    'JSON_DATA'
]


JSON_DATA = {
    "options": {
        "profile": "tests/fake.py",
        "filter": [],
        "exclude_filter": []
    },
    "name": "fake-tests",
    "lspci": "fake",
    "glxinfo": "fake",
    "tests": {
        "sometest": {
            "result": "pass",
            "time": 0.01
        }
    }
}


class UtilsException(Exception):
    """ An exception to be raised by utils """
    pass


@contextmanager
def resultfile():
    """ Create a stringio with some json in it and pass that as results """
    with tempfile.NamedTemporaryFile(delete=False) as output:
        json.dump(JSON_DATA, output)

    yield output

    os.remove(output.name)


@contextmanager
def with_tempfile(contents):
    """ Provides a context manager for a named tempfile

    This contextmanager creates a named tempfile, writes data into that
    tempfile, then closes it and yields the filepath. After the context is
    returned it closes and removes the tempfile.

    Arguments:
    contests -- This should be a string (unicode or str), in which case it is
                written directly into the file.

    """
    # Do not delete the tempfile as soon as it is closed
    temp = tempfile.NamedTemporaryFile(delete=False)
    temp.write(contents)
    temp.close()

    yield temp.name

    os.remove(temp.name)


@contextmanager
def tempdir():
    """ Creates a temporary directory, returns it, and then deletes it """
    tdir = tempfile.mkdtemp()
    yield tdir
    shutil.rmtree(tdir)
