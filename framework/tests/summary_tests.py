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


""" Module providing tests for the summary module """

try:
    import simplejson as json
except ImportError:
    import json
import os
import tempfile
from contextlib import contextmanager
import framework.summary as summary


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


@contextmanager
def _resultfile():
    """ Create a stringio with some json in it and pass that as results """
    with tempfile.NamedTemporaryFile(delete=False) as output:
        json.dump(JSON_DATA, output)

    yield output

    os.remove(output.name)


def test_initialize_summary():
    """ Test that Summary initializes """
    with _resultfile() as files:
        test = summary.Summary([files.name])
        assert test
