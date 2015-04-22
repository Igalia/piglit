# Copyright (c) 2014, 2015 Intel Corporation

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

"""Tests for JSON backend version updates."""

from __future__ import print_function, absolute_import, division
import os
import copy
import tempfile

try:
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt

import framework.tests.utils as utils
from framework import backends, results

# Disable some errors that cannot be fixed either because tests need to probe
# protected members, or because of nose requirements, like long lines
# pylint: disable=protected-access,invalid-name,line-too-long


class TestV0toV1(object):
    """Tests for version 0 -> version 1 of json results format."""
    # NOTE: It is very important to NOT use grouptools in this class.
    # The grouptools module changes and is updated from time to time, but this file
    # must remain static. It tests the update from one static version to another
    # static version.

    @classmethod
    def setup_class(cls):
        cls.DATA = {}
        cls.DATA['options'] = {
            'profile': "tests/fake.py",
            'filter': [],
            'exclude_filter': [],
        }
        cls.DATA['name'] = 'fake-tests'
        cls.DATA['lspci'] = 'fake'
        cls.DATA['glxinfo'] = 'fake'
        cls.DATA['tests'] = {}
        cls.DATA['tests'].update({
            'sometest': {
                'result': 'pass',
                'time': 0.01,
                'dmesg': ['this', 'is', 'dmesg'],
                'info': 'Returncode: 1\n\nErrors:stderr\n\nOutput: stdout\n',
            },
            'group1/groupA/test/subtest 1': {
                'info': 'Returncode: 1\n\nErrors:stderr\n\nOutput:stdout\n',
                'subtest': {
                    'subtest 1': 'pass',
                    'subtest 2': 'pass'
                },
                'returncode': 0,
                'command': 'this is a command',
                'result': 'pass',
                'time': 0.1
            },
            'group1/groupA/test/subtest 2': {
                'info': 'Returncode: 1\n\nErrors:stderr\n\nOutput:stdout\n',
                'subtest': {
                    'subtest 1': 'pass',
                    'subtest 2': 'pass'
                },
                'returncode': 0,
                'command': 'this is a command',
                'result': 'pass',
                'time': 0.1
            },
            'single/test/thing': {
                'info': 'Returncode: 1\n\nErrors:stderr\n\nOutput:stdout\n',
                'subtest': {
                    'subtest 1': 'pass',
                },
                'returncode': 0,
                'command': 'this is a command',
                'result': 'pass',
                'time': 0.1
            },
            'group2/groupA/test/subtest 1/depth': {
                'info': 'Returncode: 1\n\nErrors:stderr\n\nOutput:stdout\n',
                'subtest': {
                    'subtest 1/depth': 'pass',
                    'subtest 2/float': 'pass'
                },
                'returncode': 0,
                'command': 'this is a command',
                'result': 'pass',
                'time': 0.1
            },
            'group2/groupA/test/subtest 2/float': {
                'info': 'Returncode: 1\n\nErrors:stderr\n\nOutput:stdout\n',
                'subtest': {
                    'subtest 1/depth': 'pass',
                    'subtest 2/float': 'pass'
                },
                'returncode': 0,
                'command': 'this is a command',
                'result': 'pass',
                'time': 0.1
            },
            'group3/groupA/test': {
                'info': 'Returncode: 1\n\nErrors:stderr\n\nOutput:stdout\n',
                'subtest': {
                    'subtest 1': 'pass',
                    'subtest 2': 'pass',
                    'subtest 3': 'pass',
                },
                'returncode': 0,
                'command': 'this is a command',
                'result': 'pass',
                'time': 0.1
            },
        })

        with utils.tempfile(json.dumps(cls.DATA)) as t:
            with open(t, 'r') as f:
                cls.RESULT = backends.json._update_zero_to_one(backends.json._load(f))

    def test_dmesg(self):
        """backends.json.update_results (0 -> 1): dmesg is converted from a list to a string"""
        assert self.RESULT.tests['sometest']['dmesg'] == 'this\nis\ndmesg'

    def test_subtests_remove_duplicates(self):
        """backends.json.update_results (0 -> 1): Removes duplicate entries"""
        assert 'group1/groupA/test/subtest 1' not in self.RESULT.tests
        assert 'group1/groupA/test/subtest 2' not in self.RESULT.tests

    def test_subtests_add_test(self):
        """backends.json.update_results (0 -> 1): Add an entry for the actual test"""
        assert self.RESULT.tests.get('group1/groupA/test')

    def test_subtests_test_is_testresult(self):
        """backends.json.update_results (0 -> 1): The result of the new test is a TestResult Instance"""
        assert isinstance(
            self.RESULT.tests['group1/groupA/test'],
            results.TestResult)

    def test_info_delete(self):
        """backends.json.update_results (0 -> 1): Remove the info name from results"""
        for value in self.RESULT.tests.itervalues():
            assert 'info' not in value

    def test_returncode_from_info(self):
        """backends.json.update_results (0 -> 1): Use the returncode from info if there is no returncode"""
        assert self.RESULT.tests['sometest']['returncode'] == 1

    def test_returncode_no_override(self):
        """backends.json.update_results (0 -> 1): Do not clobber returncode with info

        The returncode from info should not overwrite an existing returcnode
        attribute, this test only tests that the value in info isn't used when
        there is a value in returncode already

        """
        assert self.RESULT.tests['group1/groupA/test']['returncode'] != 1

    def test_err_from_info(self):
        """backends.json.update_results (0 -> 1): add an err attribute from info"""
        assert self.RESULT.tests['group1/groupA/test']['err'] == 'stderr'

    def test_out_from_info(self):
        """backends.json.update_results (0 -> 1): add an out attribute from info"""
        assert self.RESULT.tests['group1/groupA/test']['out'] == 'stdout'

    def test_set_version(self):
        """backends.json.update_results (0 -> 1): Set the version to 1"""
        assert self.RESULT.results_version == 1

    def test_dont_break_single_subtest(self):
        """backends.json.update_results (0 -> 1): Don't break single subtest entries

        A test with a single subtest was written correctly before, dont break it by
        removing the name of the test. ex:
        test/foo/bar: {
            ...
            subtest: {
                1x1: pass
            }
        }

        should remain test/foo/bar since bar is the name of the test not a subtest

        """
        assert self.RESULT.tests['single/test/thing']

    def test_subtests_with_slash(self):
        """backends.json.update_results (0 -> 1): Subtest names with /'s are handled correctly"""

        expected = 'group2/groupA/test/subtest 1'
        nt.assert_not_in(
            expected, self.RESULT.tests.iterkeys(),
            msg='{0} found in result, when it should not be'.format(expected))

    def test_handle_fixed_subtests(self):
        """backends.json.update_results (0 -> 1): Correctly handle new single entry subtests correctly"""
        assert 'group3/groupA/test' in self.RESULT.tests.iterkeys()

    def _load_with_update(self, data=None):
        """If the file is not results.json, it will be renamed.

        This ensures that the right file is removed.

        """
        if not data:
            data = self.DATA

        try:
            with utils.tempfile(json.dumps(data)) as t:
                result = backends.json.load_results(t)
        except OSError as e:
            # There is the potential that the file will be renamed. In that event
            # remove the renamed files
            if e.errno == 2:
                os.unlink(os.path.join(tempfile.tempdir, 'results.json'))
                os.unlink(os.path.join(tempfile.tempdir, 'results.json.old'))
            else:
                raise

        return result

    def test_load_results_unversioned(self):
        """backends.json.load_results: Loads unversioned results and updates correctly.

        This is just a random change to show that the update path is being hit.

        """
        result = self._load_with_update()
        nt.assert_equal(result.tests['sometest']['dmesg'], 'this\nis\ndmesg')

    def test_load_results_v0(self):
        """backends.json.load_results: Loads results v0 and updates correctly.

        This is just a random change to show that the update path is being hit.

        """
        data = copy.deepcopy(self.DATA)
        data['results_version'] = 0

        result = self._load_with_update(data)
        nt.assert_equal(result.tests['sometest']['dmesg'], 'this\nis\ndmesg')

    def test_info_split(self):
        """backends.json.update_results (0 -> 1): info can split into any number of elements"""
        data = copy.copy(self.DATA)
        data['tests']['sometest']['info'] = \
            'Returncode: 1\n\nErrors:stderr\n\nOutput: stdout\n\nmore\n\nstuff'

        with utils.tempfile(json.dumps(data)) as t:
            with open(t, 'r') as f:
                backends.json._update_zero_to_one(backends.json._load(f))


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

        with utils.tempfile(json.dumps(data)) as t:
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

        with utils.tempfile(json.dumps(data)) as t:
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


class TestV2toV3(object):
    """Tests for version 2 -> version 3 of json results"""
    # NOTE: do NOT use grouptools in this class, see v0 tests for explanation

    @classmethod
    def setup_class(cls):
        data = {
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

        with utils.tempfile(json.dumps(data)) as t:
            with open(t, 'r') as f:
                # pylint: disable=protected-access
                cls.RESULT = backends.json._update_two_to_three(backends.json._load(f))

    def test_unchanged(self):
        """backends.json.update_results (2 -> 3): results with no caps are not mangled"""
        nt.ok_('test/is/a/test' in self.RESULT.tests)

    def test_lower(self):
        """backends.json.update_results (2 -> 3): results with caps are lowered"""
        nt.ok_('test/is/some/other1/test' in self.RESULT.tests)

    def test_removed(self):
        """backends.json.update_results (2 -> 3): results with caps are removed"""
        nt.ok_('Test/Is/SomE/Other1/Test' not in self.RESULT.tests)


class TestV3toV4(object):
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
        "results_version": 3,
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
            "spec/arb_texture_rg/fs-shadow2d-red-01": TEST_DATA,
            "spec/arb_texture_rg/fs-shadow2d-red-02": TEST_DATA,
            "spec/arb_texture_rg/fs-shadow2d-red-03": TEST_DATA,
            "spec/arb_draw_instanced/draw-non-instanced": TEST_DATA,
            "spec/arb_draw_instanced/instance-array-dereference": TEST_DATA,
            "glslparsertest/foo": TEST_DATA,
        }
    }

    @staticmethod
    def _make_result(data):
        """Write data to a file and return a result.TestrunResult object."""
        with utils.tempfile(json.dumps(data)) as t:
            with open(t, 'r') as f:
                # pylint: disable=protected-access
                return backends.json._update_three_to_four(backends.json._load(f))

    @classmethod
    def setup_class(cls):
        """Class setup. Create a TestrunResult with v3 data."""
        cls.old = cls.DATA['tests'].keys()
        cls.new = [
            "spec/arb_texture_rg/execution/fs-shadow2d-red-01",
            "spec/arb_texture_rg/execution/fs-shadow2d-red-02",
            "spec/arb_texture_rg/execution/fs-shadow2d-red-03",
            "spec/arb_draw_instanced/execution/draw-non-instanced",
            "spec/arb_draw_instanced/execution/instance-array-dereference",
        ]
        cls.result = cls._make_result(cls.DATA)

    def test_old_removed(self):
        """backends.json.update_results (3 -> 4): All old test names are removed"""
        for old in self.old:
            nt.assert_not_in(old, self.result.tests)

    def test_new_added(self):
        """backends.json.update_results (3 -> 4): All new test names are added"""
        for new in self.new:
            nt.assert_in(new, self.result.tests)

    def test_new_has_data(self):
        """backends.json.update_results (3 -> 4): All new tests have expected data"""
        for new in self.new:
            nt.assert_dict_equal(self.result.tests[new], self.TEST_DATA)

    @utils.not_raises(KeyError)
    def test_missing(self):
        """backends.json.update_results (3 -> 4): updates successfully when tests to rename are not present"""
        data = copy.copy(self.DATA)
        del data['tests']['spec/arb_draw_instanced/instance-array-dereference']
        self._make_result(data)


class TestV4toV5(object):
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

    @classmethod
    def setup_class(cls):
        """Class setup. Create a TestrunResult with v4 data."""
        cls.old = cls.DATA['tests'].keys()
        cls.new = [
            "a@test@group@of@great@length",
            "has@windows",
        ]

        with utils.tempfile(json.dumps(cls.DATA)) as t:
            with open(t, 'r') as f:
                cls.result = backends.json._update_four_to_five(backends.json._load(f))

    def test_posix_removed(self):
        """backends.json.update_results (4 -> 5): / is replaced with @"""
        nt.assert_not_in('a/test/of/great/length', self.result.tests)

    def test_win_removed(self):
        """backends.json.update_results (4 -> 5): \\ is replaced with @"""
        nt.assert_not_in('has\\windows', self.result.tests)

    def test_new_added(self):
        """backends.json.update_results (4 -> 5): All new test names are added."""
        for new in self.new:
            nt.assert_in(new, self.result.tests)

    def test_new_has_data(self):
        """backends.json.update_results (4 -> 5): All new tests have expected data."""
        for new in self.new:
            nt.assert_dict_equal(self.result.tests[new], self.TEST_DATA)

    def test_load_results(self):
        """backends.json.update_results (4 -> 5): load_results properly updates."""
        with utils.tempdir() as d:
            tempfile = os.path.join(d, 'results.json')
            with open(tempfile, 'w') as f:
                json.dump(self.DATA, f)
            with open(tempfile, 'r') as f:
                result = backends.json.load_results(tempfile)
                nt.assert_equal(result.results_version, 5)
