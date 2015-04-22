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


@utils.no_error
def test_initialize_summary():
    """summary.Summary: class initializes"""
    with utils.resultfile() as tfile:
        summary.Summary([tfile.name])


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
        check_sets.description = \
            "summary.Summary: {0} -> {1} should be added to {2}".format(
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


class TestSubtestHandling(object):
    """Test Summary subtest handling."""
    @classmethod
    def setup_class(cls):
        data = copy.deepcopy(utils.JSON_DATA)
        data['tests']['with_subtests']['result'] = 'pass'

        data['tests']['with_subtests']['subtest']['subtest1'] = 'fail'
        data['tests']['with_subtests']['subtest']['subtest2'] = 'warn'
        data['tests']['with_subtests']['subtest']['subtest3'] = 'crash'
        data['tests']['is_skip']['result'] = 'skip'

        with utils.with_tempfile(json.dumps(data)) as sumfile:
            cls.summ = summary.Summary([sumfile])

    def test_subtests_are_tests(self):
        """summary.Summary: Subtests should be treated as full tests"""
        nt.assert_equal(
            self.summ.fractions['fake-tests']['with_subtests'], (0, 3),
            msg="Summary.fraction['fake-tests']['with_subtests'] should "
                "be (0, 3), but isn't")

    def test_tests_w_subtests_are_groups(self):
        """summary.Summary: Tests with subtests should be a group

        We know that the status will be 'pass' if it's not being overwritten, and
        will be 'crash' if it has. (since we set the data that way)

        """
        nt.assert_equal(
            self.summ.status['fake-tests']['with_subtests'], 'crash',
            msg="Summary.status['fake-tests']['with_subtests'] should "
                "be crash, but isn't")

    def test_removed_from_all(self):
        """summary.Summary: Tests with subtests should not be in the all results
        """
        nt.assert_not_in(
            'with_subtests', self.summ.tests['all'],
            msg="Test with subtests should have been removed from "
                "self.tests['all'], but wasn't")

    def subtest_not_skip_notrun(self):
        """summary.Summary: skips are not changed to notruns"""
        nt.eq_(
            self.summ.status['fake-tests']['is_skip'], 'skip',
            msg="Status should be skip but was changed")
