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

from framework import summary, results
import framework.tests.utils as utils
from framework.backends.json import piglit_encoder


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
    old['tests']['sometest']['__type__'] = 'TestResult'
    new['tests']['sometest']['result'] = nstat
    new['tests']['sometest']['__type__'] = 'TestResult'

    with utils.tempfile(json.dumps(old, default=piglit_encoder)) as ofile:
        with utils.tempfile(json.dumps(new, default=piglit_encoder)) as nfile:
            summ = summary.Summary([ofile, nfile])

            print(summ.tests)
            nt.assert_equal(1, len(summ.tests[set_]),
                            msg="{0} was not appended".format(set_))


class TestSubtestHandling(object):
    """Test Summary subtest handling."""
    @classmethod
    def setup_class(cls):
        data = copy.deepcopy(utils.JSON_DATA)
        data['tests']['sometest']['__type__'] = 'TestResult'
        data['tests']['with_subtests']['result'] = 'pass'
        data['tests']['with_subtests']['__type__'] = 'TestResult'

        data['tests']['with_subtests']['subtests']['subtest1'] = 'fail'
        data['tests']['with_subtests']['subtests']['subtest2'] = 'warn'
        data['tests']['with_subtests']['subtests']['subtest3'] = 'crash'
        data['tests']['is_skip']['result'] = 'skip'
        data['tests']['is_skip']['__type__'] = 'TestResult'

        with utils.tempfile(json.dumps(data, default=piglit_encoder)) as sumfile:
            cls.summ = summary.Summary([sumfile])

    def test_subtests_are_tests(self):
        """summary.Summary: Subtests should be treated as full tests"""
        nt.eq_(self.summ.fractions['fake-tests']['with_subtests'], (0, 3))

    def test_tests_w_subtests_are_groups(self):
        """summary.Summary: Tests with subtests should be a group

        We know that the status will be 'pass' if it's not being overwritten, and
        will be 'crash' if it has. (since we set the data that way)

        """
        nt.eq_(self.summ.status['fake-tests']['with_subtests'], 'crash')

    def test_removed_from_all(self):
        """summary.Summary: Tests with subtests should not be in the all results
        """
        nt.assert_not_in('with_subtests', self.summ.tests['all'])

    def subtest_not_skip_notrun(self):
        """summary.Summary: skips are not changed to notruns"""
        nt.eq_(self.summ.status['fake-tests']['is_skip'], 'skip')


def test_find_diffs_():
    """summary.find_diffs: calculates correct set of diffs"""
    res1 = results.TestrunResult()
    res1.tests['foo'] = results.TestResult('pass')
    res1.tests['bar'] = results.TestResult('fail')
    res1.tests['oink'] = results.TestResult('crash')
    res1.tests['bonk'] = results.TestResult('warn')

    res2 = results.TestrunResult()
    res2.tests['foo'] = results.TestResult('fail')
    res2.tests['bar'] = results.TestResult('pass')
    res2.tests['oink'] = results.TestResult('crash')

    diffs = summary.find_diffs([res1, res2],
                               {'foo', 'bar', 'oink', 'bonk'},
                               lambda x, y: x != y)
    nt.eq_(diffs, [{'foo', 'bar'}])
