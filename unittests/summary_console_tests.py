# Copyright (c) 2015 Intel Corporation

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

"""Tests for framework.summary.console_

Some of these tests are fickle, since we're testing the output of a summary
generator.

"""

# pylint: disable=protected-access,invalid-name

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import sys

import nose.tools as nt
import six
from six.moves import cStringIO as StringIO

from . import utils
from framework import results, grouptools
from framework.summary import console_, common


def get_stdout(callable_):
    """Capture stdout from a import callable"""
    capture = StringIO()
    temp = sys.stdout
    sys.stdout = capture
    callable_()
    sys.stdout = temp

    return capture.getvalue()


class Test_print_summary(object):
    """Tests for the console output."""
    _ENUMS = {
        'names': 1,
        'dividier': 2,
        'pass': 3,
        'fail': 4,
        'crash': 5,
        'skip': 6,
        'timeout': 7,
        'warn': 8,
        'incomplete': 9,
        'dmesg-warn': 10,
        'dmesg-fail': 11,
        'changes': 12,
        'fixes': 13,
        'regressions': 14,
        'total': 15,
    }

    @classmethod
    def setup_class(cls):
        """Create both an expected value and an actual value.

        The expected value makes use of the template, this helps to minimize
        the number of changes that need to be made if the output is altered.

        """
        names = [grouptools.join('foo', 'bar', 'oink', 'foobar', 'boink'),
                 'foo', 'bar']
        template = '{: >20.20} {: >6.6}'

        cls.expected = console_._SUMMARY_TEMPLATE.format(
            names=' '.join(['this is a really rea', 'a name']),
            divider=' '.join(['--------------------', '------']),
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
            total=template.format('3', '3')).split('\n')

        res1 = results.TestrunResult()
        res1.name = 'this is a really really really really long name'
        res1.tests[names[0]] = results.TestResult('pass')
        res1.tests[names[1]] = results.TestResult('fail')
        res1.tests[names[2]] = results.TestResult('notrun')
        res1.tests[names[2]].subtests['1'] = 'fail'
        res1.calculate_group_totals()

        res2 = results.TestrunResult()
        res2.name = 'a name'
        res2.tests[names[0]] = results.TestResult('pass')
        res2.tests[names[1]] = results.TestResult('pass')
        res2.tests[names[2]] = results.TestResult('notrun')
        res2.tests[names[2]].subtests['1'] = 'skip'
        res2.calculate_group_totals()

        reses = common.Results([res1, res2])

        cls.actual = get_stdout(
            lambda: console_._print_summary(reses)).split('\n')

    @utils.nose.generator
    def test_values(self):
        """Generate a bunch of tests."""
        def test(value):
            nt.eq_(self.actual[value], self.expected[value],
                   msg='Values do not match\n'
                       'expected "{}"\n'
                       'actual   "{}"'.format(
                           self.expected[value], self.actual[value]))

        description = "summary.console_._print_summary: calculates {} correctly"

        for key, value in six.iteritems(self._ENUMS):
            test.description = description.format(key)
            yield test, value


def test_print_result():
    """summary.console_._print_result: prints expected values"""
    res1 = results.TestrunResult()
    res1.tests['foo'] = results.TestResult('pass')

    res2 = results.TestrunResult()
    res2.tests['foo'] = results.TestResult('fail')

    reses = common.Results([res1, res2])

    expected = 'foo: pass fail\n'
    actual = get_stdout(lambda: console_._print_result(reses, reses.names.all))

    nt.eq_(expected, actual)


def test_print_result_replaces():
    """summary.console_._print_result: Replaces separtaor with /"""
    res1 = results.TestrunResult()
    res1.tests[grouptools.join('foo', 'bar')] = results.TestResult('pass')

    reses = common.Results([res1])

    expected = 'foo/bar: pass\n'
    actual = get_stdout(lambda: console_._print_result(reses, reses.names.all))

    nt.eq_(expected, actual)
