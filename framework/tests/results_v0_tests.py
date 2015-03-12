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

""" Module provides tests for converting version zero results to version 1 """

from __future__ import print_function, absolute_import
import os
import json
import copy
import tempfile

import nose.tools as nt

import framework.results as results
import framework.tests.utils as utils

# NOTE: It is very important to NOT use grouptools in this file.
# The grouptools module changes and is updated from time to time, but this file
# must remain static. It tests the update from one static version to another
# static version.

DATA = {}
DATA['options'] = {
    'profile': "tests/fake.py",
    'filter': [],
    'exclude_filter': [],
}
DATA['name'] = 'fake-tests'
DATA['lspci'] = 'fake'
DATA['glxinfo'] = 'fake'
DATA['tests'] = {}
DATA['tests'].update({
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

with utils.with_tempfile(json.dumps(DATA)) as t:
    with open(t, 'r') as f:
        RESULT = results._update_zero_to_one(results.TestrunResult.load(f))


def test_dmesg():
    """ version 1: dmesg is converted from a list to a string """
    assert RESULT.tests['sometest']['dmesg'] == 'this\nis\ndmesg'


def test_subtests_remove_duplicates():
    """ Version 1: Removes duplicate entries """
    assert 'group1/groupA/test/subtest 1' not in RESULT.tests
    assert 'group1/groupA/test/subtest 2' not in RESULT.tests


def test_subtests_add_test():
    """ Version 1: Add an entry for the actual test """
    assert RESULT.tests.get('group1/groupA/test')


def test_subtests_test_is_testresult():
    """ Version 1: The result of the new test is a TestResult Instance """
    assert isinstance(
        RESULT.tests['group1/groupA/test'],
        results.TestResult)


def test_info_delete():
    """ Version 1: Remove the info name from results """
    for value in RESULT.tests.itervalues():
        assert 'info' not in value


def test_returncode_from_info():
    """ Version 1: Use the returncode from info if there is no returncode """
    assert RESULT.tests['sometest']['returncode'] == 1


def test_returncode_no_override():
    """ Version 1: Do not clobber returncode with info

    The returncode from info should not overwrite an existing returcnode
    attribute, this test only tests that the value in info isn't used when
    there is a value in returncode already

    """
    assert RESULT.tests['group1/groupA/test']['returncode'] != 1


def test_err_from_info():
    """ Version 1: add an err attribute from info """
    assert RESULT.tests['group1/groupA/test']['err'] == 'stderr'


def test_out_from_info():
    """ Version 1: add an out attribute from info """
    assert RESULT.tests['group1/groupA/test']['out'] == 'stdout'


def test_set_version():
    """ Version 1: Set the version to 1 """
    assert RESULT.results_version == 1


def test_dont_break_single_subtest():
    """ Version 1: Don't break single subtest entries

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
    assert RESULT.tests['single/test/thing']


def test_info_split():
    """ Version 1: info can split into any number of elements """
    data = copy.copy(DATA)
    data['tests']['sometest']['info'] = \
        'Returncode: 1\n\nErrors:stderr\n\nOutput: stdout\n\nmore\n\nstuff'

    with utils.with_tempfile(json.dumps(data)) as t:
        with open(t, 'r') as f:
            results._update_zero_to_one(results.TestrunResult.load(f))


def test_subtests_with_slash():
    """ Version 1: Subtest names with /'s are handled correctly """

    expected = 'group2/groupA/test/subtest 1'
    nt.assert_not_in(
        expected, RESULT.tests.iterkeys(),
        msg='{0} found in result, when it should not be'.format(expected))


def test_handle_fixed_subtests():
    """ Version 1: Correctly handle new single entry subtests correctly """
    assert 'group3/groupA/test' in RESULT.tests.iterkeys()


def _load_with_update(data):
    """If the file is not results.json, it will be renamed.

    This ensures that the right file is removed.

    """
    try:
        with utils.with_tempfile(json.dumps(data)) as t:
            result = results.load_results(t)
    except OSError as e:
        # There is the potential that the file will be renamed. In that event
        # remove the renamed files
        if e.errno == 2:
            os.unlink(os.path.join(tempfile.tempdir, 'results.json'))
            os.unlink(os.path.join(tempfile.tempdir, 'results.json.old'))
        else:
            raise

    return result


def test_load_results_unversioned():
    """results.load_results: Loads unversioned results and updates correctly.

    This is just a random change to show that the update path is being hit.

    """
    result = _load_with_update(DATA)
    nt.assert_equal(result.tests['sometest']['dmesg'], 'this\nis\ndmesg')


def test_load_results_v0():
    """results.load_results: Loads results v0 and updates correctly.

    This is just a random change to show that the update path is being hit.

    """
    data = copy.deepcopy(DATA)
    data['results_version'] = 0

    result = _load_with_update(data)
    nt.assert_equal(result.tests['sometest']['dmesg'], 'this\nis\ndmesg')
