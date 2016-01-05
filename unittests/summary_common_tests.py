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

from __future__ import absolute_import, division, print_function
import datetime

import nose.tools as nt

from framework import results, status, grouptools
from framework.summary import common as summary
from . import utils


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
        res1.tests['red'] = results.TestResult('skip')

        res2 = results.TestrunResult()
        res2.tests['foo'] = results.TestResult('fail')
        res2.tests['bar'] = results.TestResult('pass')
        res2.tests['oink'] = results.TestResult('crash')
        res2.tests['tonk'] = results.TestResult('incomplete')
        res2.tests['red'] = results.TestResult('pass')

        cls.test = summary.Results([res1, res2])

    def test_names_all(self):
        """summary.Names.all: contains a set of all tests"""
        baseline = {'foo', 'bar', 'oink', 'bonk', 'bor', 'tonk', 'red'}
        nt.assert_set_equal(self.test.names.all, baseline)

    def test_names_changes_all(self):
        """summary.Names.all_changes: contains a set of all changed tests"""
        nt.assert_set_equal(self.test.names.all_changes,
                            {'foo', 'bar', 'tonk', 'red', 'bonk'})

    def test_names_problems_all(self):
        """summary.Names.all_problems: contains a set of all problems"""
        baseline = {'bar', 'oink', 'bonk', 'foo', 'tonk'}
        nt.eq_(self.test.names.all_problems, baseline)

    def test_names_skips_all(self):
        """summary.Names.all_skips: contains a set of all skips"""
        nt.eq_(self.test.names.all_skips, {'bor', 'red'})

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
        nt.assert_set_equal(self.test.names.changes[1],
                            {'foo', 'bar', 'tonk', 'red', 'bonk'})

    def test_names_problems(self):
        """summary.Names.problems: contains a list of problems in each run"""
        baseline = [{'bar', 'oink', 'bonk'}, {'foo', 'oink', 'tonk'}]
        nt.eq_(self.test.names.problems, baseline)

    def test_names_skips(self):
        """summary.Names.skips: contains a list of skips in each run"""
        baseline = [{'bor', 'red'}, set()]
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
        nt.eq_(self.test.counts.all, 7)

    def test_counts_changes(self):
        """summary.Counts.changes: contains a list of the number of differences between results"""
        # We can safely throw away the first value, since it's a padding value
        # (since nothing comes before the first value it can't have changes)
        nt.eq_(self.test.counts.changes[1], 5)

    def test_counts_problems(self):
        """summary.Counts.problems: contains a list of the number of problems in each run"""
        nt.eq_(self.test.counts.problems, [3, 3])

    def test_counts_skips(self):
        """summary.Counts.skips: contains a list of skips in each run"""
        nt.eq_(self.test.counts.skips, [2, 0])

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
        res1.tests['bor'] = results.TestResult('crash')
        res1.tests['bor'].subtests['1'] = 'pass'
        res1.tests['bor'].subtests['2'] = 'skip'
        res1.tests['bor'].subtests['3'] = 'fail'
        res1.tests['bor'].subtests['4'] = 'pass'

        res2 = results.TestrunResult()
        res2.tests['foo'] = results.TestResult('fail')
        res2.tests['bar'] = results.TestResult('pass')
        res2.tests['oink'] = results.TestResult('crash')
        res2.tests['tonk'] = results.TestResult('incomplete')
        res2.tests['bor'] = results.TestResult('crash')
        res2.tests['bor'].subtests['1'] = 'fail'
        res2.tests['bor'].subtests['2'] = 'skip'
        res2.tests['bor'].subtests['3'] = 'pass'
        res2.tests['bor'].subtests['5'] = 'pass'

        cls.test = summary.Results([res1, res2])

    def test_all(self):
        """summary.Names.all: Handles subtests as groups"""
        baseline = {'foo', 'bar', 'oink', 'bonk', 'oink', 'tonk'}
        for x in xrange(1, 6):
            baseline.add(grouptools.join('bor', str(x)))

        nt.eq_(self.test.names.all, baseline)

    def test_sublcass_names(self):
        """summary.Names.all: subtest in all"""
        expected = grouptools.join('bor', '1')
        source = self.test.names.all

        nt.ok_(expected in source,
               msg='{} not found in {}'.format(expected, source))

    @utils.nose_generator
    def test_gen_tests(self):
        """generate a bunch of different tests."""
        def test(attr):
            expected = grouptools.join('bor', '1')
            source = getattr(self.test.names, attr)[1]

            nt.ok_(expected in source,
                   msg='{} not found in "{}"'.format(expected, source))

        desc = 'summary.Names.{}: tests added properly'
        for attr in ['changes', 'problems', 'regressions']:
            test.description = desc.format(attr)
            yield test, attr

    def test_names_fixes(self):
        """summary.Names.fixes: subtests added properly"""
        expected = grouptools.join('bor', '3')
        source = self.test.names.fixes[1]

        nt.ok_(expected in source,
               msg='{} not found in "{}"'.format(expected, source))

    def test_names_skips(self):
        """summary.Names.skips: subtests added properly"""
        expected = grouptools.join('bor', '2')
        source = self.test.names.skips[1]

        nt.ok_(expected in source,
               msg='{} not found in "{}"'.format(expected, source))

    def test_names_enabled(self):
        """summary.Names.enabled: subtests added properly"""
        expected = grouptools.join('bor', '5')
        source = self.test.names.enabled[1]

        nt.ok_(expected in source,
               msg='{} not found in "{}"'.format(expected, source))

    def test_names_disabled(self):
        """summary.Names.disabled: subtests added properly"""
        expected = grouptools.join('bor', '4')
        source = self.test.names.disabled[1]

        nt.ok_(expected in source,
               msg='{} not found in "{}"'.format(expected, source))


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


def test_escape_filename():
    """summary.escape_filename: replaces invalid characters with '_'"""
    invalid = r'<>:"|?*#'
    expected = '_' * len(invalid)

    nt.eq_(expected, summary.escape_filename(invalid))


def test_escape_pathname():
    """summary.escape_pathname: replaces '/' and '\\' with '_'"""
    invalid = 'foo/bar\\boink'
    expected = 'foo_bar_boink'

    nt.eq_(expected, summary.escape_pathname(invalid))
