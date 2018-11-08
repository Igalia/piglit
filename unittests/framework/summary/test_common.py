# coding=utf-8
# Copyright (c) 2014, 2016 Intel Corporation

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


"""Tests for framework/summary/common.py"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import pytest
from six.moves import range

from framework import grouptools
from framework import results
from framework import status
from framework.summary import common as summary

# pylint: disable=no-self-use


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
    assert diffs == [{'foo', 'bar'}]


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
    assert diffs == [{'oink', 'bonk', 'bar'}, {'foo', 'oink'}]


class TestResults(object):
    """Tests for the Results class."""

    @pytest.fixture(scope='module')
    def test(self):
        """Set some values to be used by all tests."""
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

        return summary.Results([res1, res2])

    class TestNames(object):
        """Tests for the names attribute."""

        def test_all(self, test):
            """summary.Names.all: contains a set of all tests."""
            baseline = {'foo', 'bar', 'oink', 'bonk', 'bor', 'tonk', 'red'}
            assert test.names.all == baseline

        def test_changes_all(self, test):
            """summary.Names.all_changes: contains a set of all changed tests.
            """
            assert test.names.all_changes == \
                {'foo', 'bar', 'tonk', 'red', 'bonk'}

        def test_problems_all(self, test):
            """summary.Names.all_problems: contains a set of all problems."""
            baseline = {'bar', 'oink', 'bonk', 'foo', 'tonk'}
            assert test.names.all_problems == baseline

        def test_skips_all(self, test):
            """summary.Names.all_skips: contains a set of all skips."""
            assert test.names.all_skips == {'bor', 'red'}

        def test_regressions_all(self, test):
            """summary.Names.all_regressions: contains a set of all
            regressions."""
            assert test.names.all_regressions == {'foo'}

        def test_fixes_all(self, test):
            """summary.Names.all_fixes: contains a set of all fixes."""
            # See comment in test_changes
            assert test.names.all_fixes == {'bar'}

        def test_enabled_all(self, test):
            """summary.Names.all_enabled: contains a set of all enabled tests.
            """
            assert test.names.all_enabled == {'tonk'}

        def test_disabled_all(self, test):
            """summary.Names.all_disabled: contains a set of all disabled
            tests.
            """
            assert test.names.all_disabled == {'bonk', 'bor'}

        def test_incomplete_all(self, test):
            """summary.Names.all_incomplete: contains a set of all incomplete
            tests.
            """
            assert test.names.all_incomplete == {'tonk'}

        def test_changes(self, test):
            """summary.Names.changes: contains a list of differences between
            results.
            """
            # We can safely throw away the first value, since it's a padding
            # value (since nothing comes before the first value it can't have
            # changes)
            assert test.names.changes[1] == \
                {'foo', 'bar', 'tonk', 'red', 'bonk'}

        def test_problems(self, test):
            """summary.Names.problems: contains a list of problems in each run.
            """
            baseline = [{'bar', 'oink', 'bonk'}, {'foo', 'oink', 'tonk'}]
            assert test.names.problems == baseline

        def test_skips(self, test):
            """summary.Names.skips: contains a list of skips in each run."""
            baseline = [{'bor', 'red'}, set()]
            assert test.names.skips == baseline

        def test_regressions(self, test):
            """summary.Names.regressions: contains a list of regressions
            between results.
            """
            # See comment in test_changes
            assert test.names.regressions[1] == {'foo'}

        def test_fixes(self, test):
            """summary.Names.fixes: contains a list of fixes between results.
            """
            # See comment in test_changes
            assert test.names.fixes[1] == {'bar'}

        def test_enabled(self, test):
            """summary.Names.enabled: contains a list of tests enabled between
            results.
            """
            # See comment in test_changes
            assert test.names.enabled[1] == {'tonk'}

        def test_disabled(self, test):
            """summary.Names.enagled: contains a list of tests disabled between
            results.
            """
            # See comment in test_changes
            assert test.names.disabled[1] == {'bonk', 'bor'}

        def test_incomplete(self, test):
            """summary.Names.incomplete: contains a list of incompletes in each
            run.
            """
            baseline = [set(), {'tonk'}]
            assert test.names.incomplete == baseline

    class TestCounts(object):
        """Tests for the counts attribute."""

        def test_all(self, test):
            """summary.Counts.all: Is the total number of unique tests"""
            assert test.counts.all == 7

        def test_changes(self, test):
            """summary.Counts.changes: contains a list of the number of
            differences between results.
            """
            # We can safely throw away the first value, since it's a padding
            # value (since nothing comes before the first value it can't have
            # changes)
            assert test.counts.changes[1] == 5

        def test_problems(self, test):
            """summary.Counts.problems: contains a list of the number of
            problems in each run.
            """
            assert test.counts.problems == [3, 3]

        def test_skips(self, test):
            """summary.Counts.skips: contains a list of skips in each run"""
            assert test.counts.skips == [2, 0]

        def test_regressions(self, test):
            """summary.Counts.regressions: contains a list of the number of
            regressions between results.
            """
            # See comment in test_changes
            assert test.counts.regressions[1] == 1

        def test_fixes(self, test):
            """summary.Counts.fixes: contains a list of the number of fixes
            between results.
            """
            # See comment in test_changes
            assert test.counts.fixes[1] == 1

        def test_enabled(self, test):
            """summary.Counts.enagled: contains a list of the number of tests
            enabled between results.
            """
            # See comment in test_changes
            assert test.counts.enabled[1] == 1

        def test_disabled(self, test):
            """summary.Counts.enagled: contains a list of the number of tests
            disabled between results.
            """
            # See comment in test_changes
            assert test.counts.disabled[1] == 2

        def test_incomplete(self, test):
            """summary.Counts.skips: contains a list of the number of skips in
            each run.
            """
            assert test.counts.incomplete == [0, 1]

    class TestGetResults(object):
        """Tests for the get_results method."""

        @pytest.fixture(scope='class')
        def result(self):
            """Returns a result object with no subtests."""
            res1 = results.TestrunResult()
            res1.tests['foo'] = results.TestResult('pass')

            res2 = results.TestrunResult()
            res2.tests['foo'] = results.TestResult('fail')
            res2.tests['bar'] = results.TestResult('fail')

            return summary.Results([res1, res2])

        def test_basic(self, result):
            """summary.Results.get_results: returns list of statuses."""
            assert result.get_result('foo') == [status.PASS, status.FAIL]

        def test_missing(self, result):
            """summary.Results.get_results: handles KeyErrors"""
            assert result.get_result('bar') == [status.NOTRUN, status.FAIL]

        @pytest.fixture(scope='class')
        def subtest(self):
            """results a Result object with subtests."""
            res1 = results.TestrunResult()
            res1.tests['foo'] = results.TestResult('notrun')
            res1.tests['foo'].subtests['1'] = 'pass'
            res1.tests['bar'] = results.TestResult('notrun')
            res1.tests['bar'].subtests['1'] = 'pass'

            res2 = results.TestrunResult()
            res2.tests['foo'] = results.TestResult('notrun')
            res2.tests['foo'].subtests['1'] = 'fail'

            return summary.Results([res1, res2])

        def test_subtest(self, subtest):
            """summary.Results.get_results (subtest): returns list of statuses.
            """
            assert subtest.get_result(grouptools.join('foo', '1')) == \
                [status.PASS, status.FAIL]

        def test_missing_subtest(self, subtest):
            """summary.Results.get_results (subtest): handles KeyErrors"""
            assert subtest.get_result(grouptools.join('bar', '1')) == \
                [status.PASS, status.NOTRUN]


class TestNames(object):
    """Tests for the Names object."""

    class TestNamesSubtests(object):
        """summary.Names: treats tests with subtests as groups."""

        @classmethod
        def setup_class(cls):
            """class fixture."""
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
            for x in range(1, 6):
                baseline.add(grouptools.join('bor', str(x)))

            assert self.test.names.all == baseline

        def test_sublcass_names(self):
            """summary.Names.all: subtest in all"""
            expected = grouptools.join('bor', '1')
            source = self.test.names.all

            assert expected in source

        @pytest.mark.parametrize('attr', ['changes', 'problems', 'regressions'])
        def test_bad_status(self, attr):
            """Test each of the groups of bad statuses."""
            expected = grouptools.join('bor', '1')
            source = getattr(self.test.names, attr)[1]
            assert expected in source

        def test_names_fixes(self):
            """summary.Names.fixes: subtests added properly"""
            expected = grouptools.join('bor', '3')
            source = self.test.names.fixes[1]

            assert expected in source

        def test_names_skips(self):
            """summary.Names.skips: subtests added properly"""
            expected = grouptools.join('bor', '2')
            source = self.test.names.skips[1]

            assert expected in source

        def test_names_enabled(self):
            """summary.Names.enabled: subtests added properly"""
            expected = grouptools.join('bor', '5')
            source = self.test.names.enabled[1]

            assert expected in source

        def test_names_disabled(self):
            """summary.Names.disabled: subtests added properly"""
            expected = grouptools.join('bor', '4')
            source = self.test.names.disabled[1]

            assert expected in source

    class TestNamesSingle(object):
        """Test names values that have special behavior with only one result.
        """

        @classmethod
        def setup_class(cls):
            """Class fixture."""
            res = results.TestrunResult()
            res.tests['foo'] = results.TestResult('fail')
            res.tests['bar'] = results.TestResult('pass')
            res.tests['oink'] = results.TestResult('skip')
            res.tests['tonk'] = results.TestResult('incomplete')

            cls.test = summary.Results([res])

        @pytest.mark.parametrize("attr", [
            'changes', 'disabled', 'enabled', 'fixes', 'regressions'])
        def test_compare(self, attr):
            """Test statuses that require comparisons.

            These should always return an empty set.
            """
            assert getattr(self.test.names, 'all_' + attr) == set()

        @pytest.mark.parametrize("attr", ['incomplete', 'problems', 'skips'])
        def test_index_zero(self, attr):
            """Test status that return the non-all value in index 0.

            ie: names.all_skips == names.skips[0].
            """
            assert getattr(self.test.names, 'all_' + attr) == \
                getattr(self.test.names, attr)[0]


class TestEscapeFilename(object):
    """Tests for the escape_filename function."""

    def test_basic(self):
        """summary.escape_filename: replaces invalid characters with '_.'"""
        invalid = r'<>:"|?*#'
        expected = '_' * len(invalid)

        assert expected == summary.escape_filename(invalid)

    def test_pathname(self):
        """summary.escape_pathname: replaces '/' and '\\' with '_.'"""
        invalid = 'foo/bar\\boink'
        expected = 'foo_bar_boink'

        assert expected == summary.escape_pathname(invalid)
