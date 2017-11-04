# Copyright (c) 2015-2016 Intel Corporation

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

"""Tests for framework.summary.console_.

Some of these tests are fickle, since we're testing the output of a summary
generator.
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import pytest
import six

from framework import results
from framework import grouptools
from framework.summary import console_, common

# pylint: disable=no-self-use,protected-access

_ENUMS = {
    1: 'names',
    2: 'divider',
    3: 'pass',
    4: 'fail',
    5: 'crash',
    6: 'skip',
    7: 'timeout_',  # "timeout" interferes with py.test's dependency injection
    8: 'warn',
    9: 'incomplete',
    10: 'dmesg-warn',
    11: 'dmesg-fail',
    12: 'changes',
    13: 'fixes',
    14: 'regressions',
    15: 'total',
    16: 'time',
}

class TestPrintSummary(object):
    """Tests for the console output."""

    @pytest.mark.parametrize("index", six.iterkeys(_ENUMS), ids=lambda i: _ENUMS[i])
    def test_values(self, index, capsys):
        """Create both an expected value and an actual value.

        The expected value makes use of the template, this helps to minimize
        the number of changes that need to be made if the output is altered.
        """
        names = [grouptools.join('foo', 'bar', 'oink', 'foobar', 'boink'),
                 'foo', 'bar']
        template = '{: >20.20} {: >12.12}'

        expected = console_._SUMMARY_TEMPLATE.format(
            names=' '.join(['this is a really rea', 'another name']),
            divider=' '.join(['--------------------', '------------']),
            pass_=template.format('1', '2'),
            fail=template.format('2', '0'),
            crash=template.format('0', '0'),
            skip=template.format('0', '1'),
            timeout=template.format('0', '0'),
            warn=template.format('0', '0'),
            incomplete=template.format('0', '0'),
            dmesg_warn=template.format('0', '0'),
            dmesg_fail=template.format('0', '0'),
            changes=template.format('0', '2'),
            fixes=template.format('0', '1'),
            regressions=template.format('0', '0'),
            total=template.format('3', '3'),
            time=template.format('00:01:39', '02:14:05')).split('\n')

        res1 = results.TestrunResult()
        res1.name = 'this is a really really really really long name'
        res1.tests[names[0]] = results.TestResult('pass')
        res1.tests[names[1]] = results.TestResult('fail')
        res1.tests[names[2]] = results.TestResult('notrun')
        res1.tests[names[2]].subtests['1'] = 'fail'
        res1.time_elapsed = results.TimeAttribute(1509747121.4873962, 1509747220.544042)
        res1.calculate_group_totals()

        res2 = results.TestrunResult()
        res2.name = 'another name'
        res2.tests[names[0]] = results.TestResult('pass')
        res2.tests[names[1]] = results.TestResult('pass')
        res2.tests[names[2]] = results.TestResult('notrun')
        res2.tests[names[2]].subtests['1'] = 'skip'
        res2.time_elapsed = results.TimeAttribute(1464820707.4581327, 1464828753.201948)
        res2.calculate_group_totals()

        reses = common.Results([res1, res2])

        console_._print_summary(reses)
        actual = capsys.readouterr()[0].splitlines()

        assert actual[index] == expected[index]


class TestPrintResult(object):
    """Tests for the _print_result function."""

    def test_basic(self, capsys):
        """summary.console_._print_result: prints expected values."""
        res1 = results.TestrunResult()
        res1.tests['foo'] = results.TestResult('pass')

        res2 = results.TestrunResult()
        res2.tests['foo'] = results.TestResult('fail')

        reses = common.Results([res1, res2])

        expected = 'foo: pass fail\n'
        console_._print_result(reses, reses.names.all)
        actual, _ = capsys.readouterr()

        assert expected == actual

    def test_replaces_separator(self, capsys):
        """summary.console_._print_result: Replaces separator with /."""
        res1 = results.TestrunResult()
        res1.tests[grouptools.join('foo', 'bar')] = results.TestResult('pass')

        reses = common.Results([res1])

        expected = 'foo/bar: pass\n'
        console_._print_result(reses, reses.names.all)
        actual, _ = capsys.readouterr()

        assert expected == actual
