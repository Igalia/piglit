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

from __future__ import print_function, absolute_import
import copy

try:
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt

import framework.summary as summary
import framework.tests.utils as utils


def test_initialize_summary():
    """ Test that Summary initializes """
    with utils.resultfile() as tfile:
        test = summary.Summary([tfile.name])
        assert test


@utils.nose_generator
def test_summary_add_to_set():
    """ Generate tests for testing Summary.test sets """
    old = copy.deepcopy(utils.JSON_DATA)
    new = copy.deepcopy(utils.JSON_DATA)

    for ostat, nstat, set_ in [('fail', 'pass', 'fixes'),
                               ('pass', 'warn', 'regressions'),
                               ('dmesg-fail', 'warn', 'changes'),
                               ('dmesg-warn', 'crash', 'problems'),
                               ('notrun', 'pass', 'enabled'),
                               ('skip', 'dmesg-warn', 'enabled'),
                               ('fail', 'notrun', 'disabled'),
                               ('crash', 'skip', 'disabled'),
                               ('skip', 'skip', 'skipped'),
                               ('notrun', 'fail', 'problems'),
                               ('fail', 'notrun', 'problems'),
                               ('pass', 'fail', 'problems'),
                               ('timeout', 'pass', 'fixes'),
                               ('pass', 'timeout', 'regressions'),
                               ('pass', 'timeout', 'problems')]:
        check_sets.description = "{0} -> {1} should be added to {2}".format(
                ostat, nstat, set_)

        yield check_sets, old, ostat, new, nstat, set_


def check_sets(old, ostat, new, nstat, set_):
    """ Check that the statuses are added to the correct set """
    old['tests']['sometest']['result'] = ostat
    new['tests']['sometest']['result'] = nstat

    with utils.with_tempfile(json.dumps(old)) as ofile:
        with utils.with_tempfile(json.dumps(new)) as nfile:
            summ = summary.Summary([ofile, nfile])

            print(summ.tests)
            nt.assert_equal(1, len(summ.tests[set_]),
                            msg="{0} was not appended".format(set_))


@utils.nose_generator
def test_subtest_handling():
    data = copy.deepcopy(utils.JSON_DATA)
    data['tests']['with_subtests'] = {}
    data['tests']['with_subtests']['result'] = 'pass'

    data['tests']['with_subtests']['subtest'] = {}
    data['tests']['with_subtests']['subtest']['subtest1'] = 'fail'
    data['tests']['with_subtests']['subtest']['subtest2'] = 'warn'
    data['tests']['with_subtests']['subtest']['subtest3'] = 'crash'
    data['tests']['is_skip'] = {}
    data['tests']['is_skip']['result'] = 'skip'

    with utils.with_tempfile(json.dumps(data)) as sumfile:
        summ = summary.Summary([sumfile])

        check_subtests_are_tests.description = \
            "Subtests should be treated as full tests "
        yield check_subtests_are_tests, summ

        check_tests_w_subtests_are_groups.description = \
            "Tests with subtests should be a group"
        yield check_tests_w_subtests_are_groups, summ

        test_removed_from_all.description = \
            "Tests with subtests should not be in the tests['all'] name"
        yield test_removed_from_all, summ

        subtest_not_skip_notrun.description = \
            "Skip's should not become NotRun"
        yield subtest_not_skip_notrun, summ


@nt.nottest
def check_subtests_are_tests(summary_):
    """ Subtests should be treated as full tests """
    print(summary_.fractions)
    nt.assert_equal(summary_.fractions['fake-tests']['with_subtests'], (0, 3),
        msg="Summary.fraction['fake-tests']['with_subtests'] should "
            "be (0, 3), but isn't")


@nt.nottest
def check_tests_w_subtests_are_groups(summary_):
    """ Tests with subtests should be a group

    We know that the status will be 'pass' if it's not being overwritten, and
    will be 'crash' if it has. (since we set the data that way)

    """
    print(summary_.status)
    nt.assert_equal(
        str(summary_.status['fake-tests']['with_subtests']), 'crash',
        msg="Summary.status['fake-tests']['with_subtests'] should "
            "be crash, but isn't")


@nt.nottest
def test_removed_from_all(summary_):
    """ Tests with subtests should not be in the all results """
    print(summary_.tests['all'])
    nt.assert_not_in('with_subtests', summary_.tests['all'],
        msg="Test with subtests should have been removed from "
            "self.tests['all'], but wasn't")


@nt.nottest
def subtest_not_skip_notrun(summary_):
    """ Ensure that skips are not changed to notruns """
    print(summary_.status['fake-tests']['is_skip'])
    print(summary_.results[0].tests['is_skip'])
    nt.eq_(summary_.status['fake-tests']['is_skip'], 'skip',
        msg="Status should be skip but was changed")
