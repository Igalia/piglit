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

"""Tests for version 1 to version 2."""

from __future__ import print_function, absolute_import

try:
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt

from framework import backends
import framework.tests.utils as utils

# NOTE: do NOT use grouptools in this module, see v0 tests for explanation


class TestV2Update(object):
    """Test V1 to V2 of results."""
    @classmethod
    def setup_class(cls):
        data = {
            "results_version": 1,
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
                }
            }
        }

        with utils.with_tempfile(json.dumps(data)) as t:
            with open(t, 'r') as f:
                cls.result = backends.json._update_one_to_two(
                    backends.json._load(f))

    def test_version_is_two(self):
        """backends.json.update_results (1 -> 2): The result version is updated to 2"""
        nt.assert_equal(self.result.results_version, 2)

    def test_no_env(self):
        """backends.json.update_results (1 -> 2): Removes options['env']"""
        nt.ok_('env' not in self.result.options)

    def test_glxinfo(self):
        """backends.json.update_results (1 -> 2): puts glxinfo in the root"""
        nt.assert_equal(self.result.glxinfo, 'and stuff')

    def test_lspci(self):
        """backends.json.update_results (1 -> 2): puts lspci in the root"""
        nt.assert_equal(self.result.lspci, 'stuff')

    def test_uname(self):
        """backends.json.update_results (1 -> 2): puts uname in the root"""
        nt.assert_equal(self.result.uname, 'more stuff')

    def test_wglinfo(self):
        """backends.json.update_results (1 -> 2): puts wglinfo in the root"""
        nt.assert_equal(self.result.wglinfo, 'stuff')


class TestV2NoUpdate(object):
    """Test a version 1 to 2 update when version 1 was correct"""
    @classmethod
    def setup_class(cls):
        data = {
            "results_version": 1,
            "name": "test",
            "lspci": "stuff",
            "uname": "more stuff",
            "glxinfo": "and stuff",
            "wglinfo": "stuff",
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
                }
            }
        }

        with utils.with_tempfile(json.dumps(data)) as t:
            with open(t, 'r') as f:
                cls.result = backends.json._update_one_to_two(
                    backends.json._load(f))

    def test_version_is_two(self):
        """backends.json.update_results (1 -> 2) no change: The result version is updated to 2"""
        nt.assert_equal(self.result.results_version, 2)

    def test_glxinfo(self):
        """backends.json.update_results (1 -> 2) no change: doesn't clobber glxinfo"""
        nt.assert_equal(self.result.glxinfo, 'and stuff')

    def test_lspci(self):
        """backends.json.update_results (1 -> 2) no change: doesn't clobber lspci"""
        nt.assert_equal(self.result.lspci, 'stuff')

    def test_uname(self):
        """backends.json.update_results (1 -> 2) no change: doesn't clobber uname"""
        nt.assert_equal(self.result.uname, 'more stuff')

    def test_wglinfo(self):
        """backends.json.update_results (1 -> 2) no change: doesn't clobber wglinfo"""
        nt.assert_equal(self.result.wglinfo, 'stuff')
