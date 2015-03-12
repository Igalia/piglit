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

"""Tests for version 4 to version 5."""

from __future__ import print_function, absolute_import, division

import os
try:
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt

import framework.results as results
import framework.tests.utils as utils

# NOTE: do NOT use grouptools in this module, see v0 tests for explanation

TEST_DATA = {
    'returncode': 0,
    'err': None,
    'environment': None,
    'command': 'foo',
    'result': 'skip',
    'time': 0.123,
    'out': None,
}

DATA = {
    "results_version": 4,
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
        "a/test/group/of/great/length": TEST_DATA,
        "has\\windows": TEST_DATA,
    }
}


def make_result(data):
    """Write data to a file and return a result.TestrunResult object."""
    with utils.with_tempfile(json.dumps(data)) as t:
        with open(t, 'r') as f:
            # pylint: disable=protected-access
            return results._update_four_to_five(results.TestrunResult.load(f))


class TestV5(object):
    """Generate tests for each update."""
    @classmethod
    def setup_class(cls):
        """Class setup. Create a TestrunResult with v4 data."""
        cls.old = DATA['tests'].keys()
        cls.new = [
            "a@test@group@of@great@length",
            "has@windows",
        ]
        cls.result = make_result(DATA)

    def test_posix_removed(self):
        """Version 4: / is replaced with @"""
        nt.assert_not_in('a/test/of/great/length', self.result.tests)

    def test_win_removed(self):
        """Version 4: / is replaced with @"""
        nt.assert_not_in('has\\windows', self.result.tests)

    def test_new_added(self):
        """Version 4: All new test names are added."""
        for new in self.new:
            nt.assert_in(new, self.result.tests)

    def test_new_has_data(self):
        """Version 4: All new tests have expected data."""
        for new in self.new:
            nt.assert_dict_equal(self.result.tests[new], TEST_DATA)


def test_load_results():
    """Version 4: load_results properly updates."""
    with utils.tempdir() as d:
        tempfile = os.path.join(d, 'results.json')
        with open(tempfile, 'w') as f:
            json.dump(DATA, f)
        with open(tempfile, 'r') as f:
            result = results.load_results(tempfile)
            nt.assert_equal(result.results_version, 5)
