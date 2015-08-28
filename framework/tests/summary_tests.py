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

from framework import summary, results, status, grouptools
from framework.tests import utils
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


def test_find_diffs():
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


def test_find_single():
    """summary.find_singlek: filters appropriately"""
    res1 = results.TestrunResult()
    res1.tests['foo'] = results.TestResult('pass')
    res1.tests['bar'] = results.TestResult('fail')
    res1.tests['oink'] = results.TestResult('crash')
    res1.tests['bonk'] = results.TestResult('warn')

    res2 = results.TestrunResult()
    res2.tests['foo'] = results.TestResult('fail')
    res2.tests['bar'] = results.TestResult('pass')
    res2.tests['oink'] = results.TestResult('crash')

    diffs = summary.find_single([res1, res2],
                                {'foo', 'bar', 'oink', 'bonk'},
                                lambda x: x > status.PASS)
    nt.eq_(diffs, [{'oink', 'bonk', 'bar'}, {'foo', 'oink'}])


class TestResults(object):
    @classmethod
    def setup_class(cls):
        res1 = results.TestrunResult()
        res1.tests['foo'] = results.TestResult('pass')
        res1.tests['bar'] = results.TestResult('fail')
        res1.tests['oink'] = results.TestResult('crash')
        res1.tests['bonk'] = results.TestResult('warn')
        res1.tests['bor'] = results.TestResult('skip')

        res2 = results.TestrunResult()
        res2.tests['foo'] = results.TestResult('fail')
        res2.tests['bar'] = results.TestResult('pass')
        res2.tests['oink'] = results.TestResult('crash')
        res2.tests['tonk'] = results.TestResult('incomplete')

        cls.test = summary.Results([res1, res2])

    def test_names_all(self):
        """summary.Names.all: contains a set of all tests"""
        baseline = {'foo', 'bar', 'oink', 'bonk', 'bor', 'tonk'} 
        nt.assert_set_equal(self.test.names.all, baseline)

    def test_names_changes_all(self):
        """summary.Names.all_changes: contains a set of all changed tests"""
        nt.assert_set_equal(self.test.names.all_changes, {'foo', 'bar'})

    def test_names_problems_all(self):
        """summary.Names.all_problems: contains a set of all problems"""
        baseline = {'bar', 'oink', 'bonk', 'foo', 'tonk'}
        nt.eq_(self.test.names.all_problems, baseline)

    def test_names_skips_all(self):
        """summary.Names.all_skips: contains a set of all skips"""
        nt.eq_(self.test.names.all_skips, {'bor'})

    def test_names_regressions_all(self):
        """summary.Names.all_regressions: contains a set of all regressions"""
        nt.assert_set_equal(self.test.names.all_regressions, {'foo'})

    def test_names_fixes_all(self):
        """summary.Names.all_fixes: contains a set of all fixes"""
        # See comment in test_changes
        nt.assert_set_equal(self.test.names.all_fixes, {'bar'})

    def test_names_enabled_all(self):
        """summary.Names.all_enabled: contains a set of all enabled tests"""
        nt.assert_set_equal(self.test.names.all_enabled, {'tonk'})

    def test_names_disabled_all(self):
        """summary.Names.all_disabled: contains a set of all disabled tests"""
        nt.assert_set_equal(self.test.names.all_disabled, {'bonk', 'bor'})

    def test_names_incomplete_all(self):
        """summary.Names.all_incomplete: contains a set of all incomplete tests"""
        nt.eq_(self.test.names.all_incomplete, {'tonk'})

    def test_names_changes(self):
        """summary.Names.changes: contains a list of differences between results"""
        # We can safely throw away the first value, since it's a padding value
        # (since nothing comes before the first value it can't have changes)
        nt.assert_set_equal(self.test.names.changes[1], {'foo', 'bar'})

    def test_names_problems(self):
        """summary.Names.problems: contains a list of problems in each run"""
        baseline = [{'bar', 'oink', 'bonk'}, {'foo', 'oink', 'tonk'}]
        nt.eq_(self.test.names.problems, baseline)

    def test_names_skips(self):
        """summary.Names.skips: contains a list of skips in each run"""
        baseline = [{'bor'}, set()]
        nt.eq_(self.test.names.skips, baseline)

    def test_names_regressions(self):
        """summary.Names.regressions: contains a list of regressions between results"""
        # See comment in test_changes
        nt.assert_set_equal(self.test.names.regressions[1], {'foo'})

    def test_names_fixes(self):
        """summary.Names.fixes: contains a list of fixes between results"""
        # See comment in test_changes
        nt.assert_set_equal(self.test.names.fixes[1], {'bar'})

    def test_names_enabled(self):
        """summary.Names.enabled: contains a list of tests enabled between results"""
        # See comment in test_changes
        nt.assert_set_equal(self.test.names.enabled[1], {'tonk'})

    def test_names_disabled(self):
        """summary.Names.enagled: contains a list of tests disabled between results"""
        # See comment in test_changes
        nt.assert_set_equal(self.test.names.disabled[1], {'bonk', 'bor'})

    def test_names_incomplete(self):
        """summary.Names.incomplete: contains a list of incompletes in each run"""
        baseline = [set(), {'tonk'}]
        nt.eq_(self.test.names.incomplete, baseline)

    def test_counts_all(self):
        """summary.Counts.all: Is the total number of unique tests"""
        nt.eq_(self.test.counts.all, 6)

    def test_counts_changes(self):
        """summary.Counts.changes: contains a list of the number of differences between results"""
        # We can safely throw away the first value, since it's a padding value
        # (since nothing comes before the first value it can't have changes)
        nt.eq_(self.test.counts.changes[1], 2)

    def test_counts_problems(self):
        """summary.Counts.problems: contains a list of the number of problems in each run"""
        nt.eq_(self.test.counts.problems, [3, 3])

    def test_counts_skips(self):
        """summary.Counts.skips: contains a list of skips in each run"""
        nt.eq_(self.test.counts.skips, [1, 0])

    def test_counts_regressions(self):
        """summary.Counts.regressions: contains a list of the number of regressions between results"""
        # See comment in test_changes
        nt.eq_(self.test.counts.regressions[1], 1)

    def test_counts_fixes(self):
        """summary.Counts.fixes: contains a list of the number of fixes between results"""
        # See comment in test_changes
        nt.eq_(self.test.counts.fixes[1], 1)

    def test_counts_enabled(self):
        """summary.Counts.enagled: contains a list of the number of tests enabled between results"""
        # See comment in test_changes
        nt.eq_(self.test.counts.enabled[1], 1)

    def test_counts_disabled(self):
        """summary.Counts.enagled: contains a list of the number of tests disabled between results"""
        # See comment in test_changes
        nt.eq_(self.test.counts.disabled[1], 2)

    def test_counts_incomplete(self):
        """summary.Counts.skips: contains a list of the number of skips in each run"""
        nt.eq_(self.test.counts.incomplete, [0, 1])


class TestNamesSubtests(object):
    """summary.Names: treats tests with subtests as groups"""
    @classmethod
    def setup_class(cls):
        res1 = results.TestrunResult()
        res1.tests['foo'] = results.TestResult('pass')
        res1.tests['bar'] = results.TestResult('fail')
        res1.tests['oink'] = results.TestResult('crash')
        res1.tests['bonk'] = results.TestResult('warn')
        res1.tests['bor'] = results.TestResult('skip')
        res1.tests['bor'].subtests['1'] = 'skip'
        res1.tests['bor'].subtests['2'] = 'skip'
        res1.tests['bor'].subtests['3'] = 'skip'

        res2 = results.TestrunResult()
        res2.tests['foo'] = results.TestResult('fail')
        res2.tests['bar'] = results.TestResult('pass')
        res2.tests['oink'] = results.TestResult('crash')
        res2.tests['tonk'] = results.TestResult('incomplete')

        cls.test = summary.Results([res1, res2])

    def test_all(self):
        """summary.Names.all: Handles subtests as groups"""
        baseline = {'foo', 'bar', 'oink', 'bonk', 'oink', 'tonk'}
        for x in xrange(1, 4):
            baseline.add(grouptools.join('bor', str(x)))

        nt.eq_(self.test.names.all, baseline)

    def test_sublcass_name(self):
        """summary.Names.all: subtest in all"""
        nt.ok_(grouptools.join('bor', '1') in self.test.names.all)


class TestNamesSingle(object):
    """Test names values that have special behavior with only one result."""
    @classmethod
    def setup_class(cls):
        res = results.TestrunResult()
        res.tests['foo'] = results.TestResult('fail')
        res.tests['bar'] = results.TestResult('pass')
        res.tests['oink'] = results.TestResult('skip')
        res.tests['tonk'] = results.TestResult('incomplete')

        cls.test = summary.Results([res])

    @utils.nose_generator
    def test_compare_status(self):
        """Test statuses that require comparisons.

        These should always return an empty set

        """
        test = lambda x: nt.eq_(getattr(self.test.names, 'all_' + x), set())

        for each in ['changes', 'disabled', 'enabled', 'fixes', 'regressions']:
            test.description = ('summary.Names.all_{} (single result): '
                                'returns empty set'.format(each))
            yield test, each

    @utils.nose_generator
    def test_index_zero(self):
        """Test status that return the non-all value in index 0

        ie: names.all_skips == names.skips[0]

        """
        test = lambda x: nt.eq_(getattr(self.test.names, 'all_' + x),
                                getattr(self.test.names, x)[0])

        for each in ['incomplete', 'problems', 'skips']:
            test.description = ('summary.Names.all_{0} (single result): '
                                'returns names.{0}[0]'.format(each))
            yield test, each


def test_Results_get_results():
    """summary.Results.get_results: returns list of statuses"""
    res1 = results.TestrunResult()
    res1.tests['foo'] = results.TestResult('pass')

    res2 = results.TestrunResult()
    res2.tests['foo'] = results.TestResult('fail')

    res = summary.Results([res1, res2])

    nt.eq_(res.get_result('foo'), [status.PASS, status.FAIL])


def test_Results_get_results_missing():
    """summary.Results.get_results: handles KeyErrors"""
    res1 = results.TestrunResult()
    res1.tests['foo'] = results.TestResult('pass')

    res2 = results.TestrunResult()
    res2.tests['bar'] = results.TestResult('fail')

    res = summary.Results([res1, res2])

    nt.eq_(res.get_result('foo'), [status.PASS, status.NOTRUN])


def test_Results_get_results_subtest():
    """summary.Results.get_results (subtest): returns list of statuses"""
    res1 = results.TestrunResult()
    res1.tests['foo'] = results.TestResult('notrun')
    res1.tests['foo'].subtests['1'] = 'pass'

    res2 = results.TestrunResult()
    res2.tests['foo'] = results.TestResult('notrun')
    res2.tests['foo'].subtests['1'] = 'fail'

    res = summary.Results([res1, res2])

    nt.eq_(res.get_result(grouptools.join('foo', '1')),
           [status.PASS, status.FAIL])


def test_Results_get_results_missing_subtest():
    """summary.Results.get_results (subtest): handles KeyErrors"""
    res1 = results.TestrunResult()
    res1.tests['foo'] = results.TestResult('pass')
    res1.tests['foo'].subtests['1'] = 'pass'

    res2 = results.TestrunResult()
    res2.tests['bar'] = results.TestResult('fail')

    res = summary.Results([res1, res2])

    nt.eq_(res.get_result(grouptools.join('foo', '1')),
           [status.PASS, status.NOTRUN])
