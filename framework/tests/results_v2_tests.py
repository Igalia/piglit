# Copyright (c) 2015 Intel Corporation

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

"""Tests for version 2 to version 3."""

from __future__ import print_function, absolute_import, division

import os
try:
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt

import framework.results as results
import framework.tests.utils as utils

DATA = {
    "results_version": 2,
    "name": "test",
    "options": {
        "profile": ['quick'],
        "dmesg": False,
        "verbose": False,
        "platform": "gbm",
        "sync": False,
        "valgrind": False,
        "filter": [],
        "concurrent": "all",
        "test_count": 0,
        "exclude_tests": [],
        "exclude_filter": [],
        "env": {
            "lspci": "stuff",
            "uname": "more stuff",
            "glxinfo": "and stuff",
            "wglinfo": "stuff"
        }
    },
    "tests": {
        "test/is/a/test": {
            "returncode": 0,
            "err": None,
            "environment": None,
            "command": "foo",
            "result": "skip",
            "time": 0.123,
            "out": None,
        },
        "Test/Is/SomE/Other1/Test": {
            "returncode": 0,
            "err": None,
            "environment": None,
            "command": "foo",
            "result": "skip",
            "time": 0.123,
            "out": None,
        }
    }
}

with utils.with_tempfile(json.dumps(DATA)) as t:
    with open(t, 'r') as f:
        # pylint: disable=protected-access
        RESULT = results._update_two_to_three(results.TestrunResult.load(f))


def test_unchanged():
    """Version 2: results with no caps are not mangled."""
    nt.ok_('test/is/a/test' in RESULT.tests)


def test_lower():
    """Version 2: results with aps are lowered."""
    nt.ok_('test/is/some/other1/test' in RESULT.tests)
