# Copyright (c) 2014-2016 Intel Corporation

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

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import copy
try:
    import simplejson as json
except ImportError:
    import json
try:
    import mock
except ImportError:
    from unittest import mock

import jsonschema
import pytest
import six

from framework import backends
from framework import results

from . import shared

# pylint: disable=protected-access,no-self-use


@pytest.yield_fixture(autouse=True, scope='module')
def setup_module():
    with mock.patch.dict(backends.json.compression.os.environ,
                         {'PIGLIT_COMPRESSION': 'none'}):
        yield


class TestV0toV1(object):
    """Tests for version 0 -> version 1 of json results format."""

    # NOTE: It is very important to NOT use grouptools in this class.
    # The grouptools module changes and is updated from time to time, but this
    # file must remain static. It tests the update from one static version to
    # another static version.
    data = {
        'options': {
            'profile': "tests/fake.py",
            'filter': [],
            'exclude_filter': [],
        },
        'name': 'fake-tests',
        'lspci': 'fake',
        'glxinfo': 'fake',
        'tests': {
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
            }
        }
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('results.json')
        p.write(json.dumps(self.data))
        with p.open('r') as f:
            return backends.json._update_zero_to_one(backends.json._load(f))

    def test_dmesg(self, result):
        """backends.json.update_results (0 -> 1): dmesg is converted from a
        list to a string.
        """
        assert result.tests['sometest']['dmesg'] == 'this\nis\ndmesg'

    def test_subtests_remove_duplicates(self, result):
        """backends.json.update_results (0 -> 1): Removes duplicate entries"""
        assert 'group1/groupA/test/subtest 1' not in result.tests
        assert 'group1/groupA/test/subtest 2' not in result.tests

    def test_subtests_add_test(self, result):
        """backends.json.update_results (0 -> 1): Add an entry for the actual
        test.
        """
        assert result.tests.get('group1/groupA/test')

    def test_subtests_is_testresult(self, result):
        """backends.json.update_results (0 -> 1): The result of the new test is
        a dict Instance.
        """
        assert isinstance(result.tests['group1/groupA/test'], dict)

    def test_info_delete(self, result):
        """backends.json.update_results (0 -> 1): Remove the info name from
        results.
        """
        for value in six.itervalues(result.tests):
            assert 'info' not in value

    def test_returncode_from_info(self, result):
        """backends.json.update_results (0 -> 1): Use the returncode from info
        if there is no returncode.
        """
        assert result.tests['sometest']['returncode'] == 1

    def test_returncode_no_override(self, result):
        """backends.json.update_results (0 -> 1): Do not clobber returncode
        with info.

        The returncode from info should not overwrite an existing returcnode
        attribute, this test only tests that the value in info isn't used when
        there is a value in returncode already.
        """
        assert result.tests['group1/groupA/test']['returncode'] != 1

    def test_err_from_info(self, result):
        """backends.json.update_results (0 -> 1): add an err attribute from
        info.
        """
        assert result.tests['group1/groupA/test']['err'] == 'stderr'

    def test_out_from_info(self, result):
        """backends.json.update_results (0 -> 1): add an out attribute from
        info.
        """
        assert result.tests['group1/groupA/test']['out'] == 'stdout'

    def test_set_version(self, result):
        """backends.json.update_results (0 -> 1): Set the version to 1"""
        assert result.results_version == 1

    def test_dont_break_single_subtest(self, result):
        """backends.json.update_results (0 -> 1): Don't break single subtest
        entries.

        A test with a single subtest was written correctly before, don't break
        it by removing the name of the test. ex:
        test/foo/bar: {
            ...
            subtest: {
                1x1: pass
            }
        }

        should remain test/foo/bar since bar is the name of the test not a
        subtest
        """
        assert result.tests.get('single/test/thing')

    def test_subtests_with_slash(self, result):
        """backends.json.update_results (0 -> 1): Subtest names with /'s are
        handled correctly.
        """
        expected = 'group2/groupA/test/subtest 1'
        assert expected not in result.tests

    def test_handle_fixed_subtests(self, result):
        """backends.json.update_results (0 -> 1): Correctly handle new single
        entry subtests correctly.
        """
        assert 'group3/groupA/test' in result.tests

    def test_load_results_unversioned(self, tmpdir):
        """backends.json.load_results: Loads unversioned results and updates .
        correctly.

        This test pickes on attribute (dmesg) to test, with the assumption taht
        if the other tests work then once the update path starts it runs
        correctly.
        """
        p = tmpdir.join('results.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        result = backends.json.load_results(six.text_type(p), 'none')

        assert result.tests['sometest'].dmesg == 'this\nis\ndmesg'

    def test_load_results_v0(self, tmpdir):
        """backends.json.load_results: Loads results v0 and updates correctly.

        This test pickes on attribute (dmesg) to test, with the assumption taht
        if the other tests work then once the update path starts it runs
        correctly.
        """
        data = copy.deepcopy(self.data)
        data['results_version'] = 0

        p = tmpdir.join('results.json')
        p.write(json.dumps(data, default=backends.json.piglit_encoder))
        result = backends.json.load_results(six.text_type(p), 'none')

        assert result.tests['sometest'].dmesg == 'this\nis\ndmesg'

    def test_info_split(self, tmpdir):
        """backends.json.update_results (0 -> 1): info can split into any
        number of elements.
        """
        data = copy.copy(self.data)
        data['tests']['sometest']['info'] = \
            'Returncode: 1\n\nErrors:stderr\n\nOutput: stdout\n\nmore\n\nstuff'

        p = tmpdir.join('results.json')
        p.write(json.dumps(data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            backends.json._update_zero_to_one(backends.json._load(f))

    def test_load_results(self, tmpdir):
        """backends.json.update_results (1 -> current): load_results properly
        updates.
        """
        p = tmpdir.join('results.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        result = backends.json.load_results(six.text_type(p), 'none')
        assert result.results_version == backends.json.CURRENT_JSON_VERSION  # pylint: disable=no-member


class TestV1toV2(object):
    """Tests version 1 to version 2."""

    class TestWithChanges(object):
        """Test V1 to V2 of results."""
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

        @pytest.fixture
        def result(self, tmpdir):
            p = tmpdir.join('result.json')
            p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
            with p.open('r') as f:
                return backends.json._update_one_to_two(backends.json._load(f))

        def test_version_is_two(self, result):
            """backends.json.update_results (1 -> 2): The result version is updated
            to 2.
            """
            assert result.results_version == 2

        def test_no_env(self, result):
            """backends.json.update_results (1 -> 2): Removes options['env']."""
            assert 'env' not in result.options

        def test_glxinfo(self, result):
            """backends.json.update_results (1 -> 2): puts glxinfo in the root."""
            assert result.glxinfo == 'and stuff'

        def test_lspci(self, result):
            """backends.json.update_results (1 -> 2): puts lspci in the root."""
            assert result.lspci == 'stuff'

        def test_uname(self, result):
            """backends.json.update_results (1 -> 2): puts uname in the root."""
            assert result.uname == 'more stuff'

        def test_wglinfo(self, result):
            """backends.json.update_results (1 -> 2): puts wglinfo in the root."""
            assert result.wglinfo == 'stuff'

    class TestWithoutChanges(object):
        """Test a version 1 to 2 update when version 1 was correct"""
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

        @pytest.fixture
        def result(self, tmpdir):
            p = tmpdir.join('result.json')
            p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
            with p.open('r') as f:
                return backends.json._update_one_to_two(backends.json._load(f))

        def test_version_is_two(self, result):
            """backends.json.update_results (1 -> 2) no change: The result version
            is updated to 2.
            """
            assert result.results_version == 2

        def test_glxinfo(self, result):
            """backends.json.update_results (1 -> 2) no change: doesn't clobber
            glxinfo.
            """
            assert result.glxinfo == 'and stuff'

        def test_lspci(self, result):
            """backends.json.update_results (1 -> 2) no change: doesn't clobber
            lspci.
            """
            assert result.lspci == 'stuff'

        def test_uname(self, result):
            """backends.json.update_results (1 -> 2) no change: doesn't clobber
            uname.
            """
            assert result.uname == 'more stuff'

        def test_wglinfo(self, result):
            """backends.json.update_results (1 -> 2) no change: doesn't clobber
            wglinfo.
            """
            assert result.wglinfo == 'stuff'


class TestV2toV3(object):
    """Tests for version 2 -> version 3 of json results"""
    # NOTE: do NOT use grouptools in this class, see v0 tests for explanation
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

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_two_to_three(backends.json._load(f))

    def test_unchanged(self, result):
        """backends.json.update_results (2 -> 3): results with no caps are not
        mangled.
        """
        assert 'test/is/a/test' in result.tests

    def test_lower(self, result):
        """backends.json.update_results (2 -> 3): results with caps are
        lowered.
        """
        assert 'test/is/some/other1/test' in result.tests

    def test_removed(self, result):
        """backends.json.update_results (2 -> 3): results with caps are
        removed.
        """
        assert 'Test/Is/SomE/Other1/Test' not in result.tests


class TestV3toV4(object):
    """Tests for version 3 to version 4."""

    # NOTE: do NOT use grouptools in this module, see v0 tests for explanation
    test_data = {
        'returncode': 0,
        'err': None,
        'environment': None,
        'command': 'foo',
        'result': 'skip',
        'time': 0.123,
        'out': None,
    }

    data = {
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
            "spec/arb_texture_rg/fs-shadow2d-red-01": test_data,
            "spec/arb_texture_rg/fs-shadow2d-red-02": test_data,
            "spec/arb_texture_rg/fs-shadow2d-red-03": test_data,
            "spec/arb_draw_instanced/draw-non-instanced": test_data,
            "spec/arb_draw_instanced/instance-array-dereference": test_data,
            "glslparsertest/foo": test_data,
        }
    }

    old = list(data['tests'].keys())
    new = [
        "spec/arb_texture_rg/execution/fs-shadow2d-red-01",
        "spec/arb_texture_rg/execution/fs-shadow2d-red-02",
        "spec/arb_texture_rg/execution/fs-shadow2d-red-03",
        "spec/arb_draw_instanced/execution/draw-non-instanced",
        "spec/arb_draw_instanced/execution/instance-array-dereference",
    ]

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_three_to_four(backends.json._load(f))

    def test_old_removed(self, result):
        """backends.json.update_results (3 -> 4): All old test names are
        removed.
        """
        for old in self.old:
            assert old not in result.tests

    def test_new_added(self, result):
        """backends.json.update_results (3 -> 4): All new test names are added.
        """
        for new in self.new:
            assert new in result.tests

    def test_new_has_data(self, result):
        """backends.json.update_results (3 -> 4): All new tests have expected
        data.
        """
        for new in self.new:
            assert result.tests[new] == self.test_data

    def test_missing(self, tmpdir):
        """backends.json.update_results (3 -> 4): updates successfully when
        tests to rename are not present.
        """
        data = copy.copy(self.data)
        del data['tests']['spec/arb_draw_instanced/instance-array-dereference']

        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))

        with p.open('r') as f:
            backends.json._update_three_to_four(backends.json._load(f))


class TestV4toV5(object):
    test_data = {
        'returncode': 0,
        'err': None,
        'environment': None,
        'command': 'foo',
        'result': 'skip',
        'time': 0.123,
        'out': None,
    }

    data = {
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
            "a/test/group/of/great/length": test_data,
            "has\\windows": test_data,
        }
    }

    old = list(data['tests'].keys())
    new = [
        'a@test@group@of@great@length',
        'has@windows',
    ]

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_four_to_five(backends.json._load(f))

    def test_posix_removed(self, result):
        """backends.json.update_results (4 -> 5): / is replaced with @."""
        assert 'a/test/of/great/length' not in result.tests

    def test_win_removed(self, result):
        """backends.json.update_results (4 -> 5): \\ is replaced with @."""
        assert 'has\\windows' not in result.tests

    def test_new_added(self, result):
        """backends.json.update_results (4 -> 5): All new test names are added.
        """
        for new in self.new:
            assert new in result.tests

    def test_new_has_data(self, result):
        """backends.json.update_results (4 -> 5): All new tests have expected
        data.
        """
        for new in self.new:
            assert result.tests[new] == self.test_data


class TestV5toV6(object):
    test_data = {
        'returncode': 0,
        'err': '',
        'environment': None,
        'command': 'foo',
        'result': 'skip',
        'time': 0.123,
        'out': '',
    }

    data = {
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
            'a@test': test_data,
        }
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_five_to_six(backends.json._load(f))

    def test_result_is_testresult_instance(self, result):
        """backends.json.update_results (5 -> 6): A test result is converted to
        a TestResult instance.
        """
        assert isinstance(result.tests['a@test'], results.TestResult)


class TestV6toV7(object):
    """Tests for version 6 to version 7."""

    data = {
        "results_version": 6,
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
            'a@test': results.TestResult('pass'),
            'a@nother@test': results.TestResult('fail'),
            'a@nother@thing': results.TestResult('crash'),
        }
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_six_to_seven(backends.json._load(f))

    def test_is_TestrunResult(self, result):
        """backends.json.update_results (6 -> 7): makes TestrunResult."""
        assert isinstance(result, results.TestrunResult)

    def test_totals(self, result):
        """backends.json.update_results (6 -> 7): Totals are populated."""
        assert result.totals != {}


class TestV7toV8(object):
    """Tests for Version 7 to version 8."""

    data = {
        "results_version": 7,
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
            'a@test': {
                'time': 1.2,
                'dmesg': '',
                'result': 'fail',
                '__type__': 'TestResult',
                'command': '/a/command',
                'traceback': None,
                'out': '',
                'environment': 'A=variable',
                'returncode': 0,
                'err': '',
                'pid': 5,
                'subtests': {
                    '__type__': 'Subtests',
                },
                'exception': None,
            }
        },
        "time_elapsed": 1.2,
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_seven_to_eight(backends.json._load(f))

    def test_time(self, result):
        """backends.json.update_results (7 -> 8): test time is stored as start
        and end.
        """
        assert result.tests['a@test'].time.start == 0.0
        assert result.tests['a@test'].time.end == 1.2

    def test_time_inst(self, result):
        """backends.json.update_results (7 -> 8): test time is a TimeAttribute
        instance.
        """
        assert isinstance(result.tests['a@test'].time, results.TimeAttribute)

    def test_time_elapsed_inst(self, result):
        """backends.json.update_results (7 -> 8): total time is stored as
        TimeAttribute.
        """
        assert isinstance(result.time_elapsed, results.TimeAttribute)

    def test_time_elapsed(self, result):
        """backends.json.update_results (7 -> 8): total time is stored as start
        and end.
        """
        assert result.time_elapsed.start == 0.0
        assert result.time_elapsed.end == 1.2

    def test_valid(self, result):
        with open(os.path.join(os.path.dirname(__file__), 'schema',
                               'piglit-8.json'),
                  'r') as f:
            schema = json.load(f)
        jsonschema.validate(
            json.loads(json.dumps(result, default=backends.json.piglit_encoder)),
            schema)


class TestV8toV9(object):
    """Tests for Version 8 to version 9."""

    data = {
        "results_version": 9,
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
            'a@test': {
                "time_elapsed": {
                    'start': 1.2,
                    'end': 1.8,
                    '__type__': 'TimeAttribute'
                },
                'dmesg': '',
                'result': 'fail',
                '__type__': 'TestResult',
                'command': '/a/command',
                'traceback': None,
                'out': '',
                'environment': 'A=variable',
                'returncode': 0,
                'err': '',
                'pid': 5,
                'subtests': {
                    '__type__': 'Subtests',
                },
                'exception': None,
            }
        },
        "time_elapsed": {
            'start': 1.2,
            'end': 1.8,
            '__type__': 'TimeAttribute'
        }
    }

    @pytest.fixture
    def result(self, tmpdir):
        p = tmpdir.join('result.json')
        p.write(json.dumps(self.data, default=backends.json.piglit_encoder))
        with p.open('r') as f:
            return backends.json._update_eight_to_nine(backends.json._load(f))

    def test_pid(self, result):
        assert result.tests['a@test'].pid == [5]

    def test_valid(self, result):
        with open(os.path.join(os.path.dirname(__file__), 'schema',
                               'piglit-9.json'),
                  'r') as f:
            schema = json.load(f)
        jsonschema.validate(
            json.loads(json.dumps(result, default=backends.json.piglit_encoder)),
            schema)
