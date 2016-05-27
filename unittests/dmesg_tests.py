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

"""Tests for the dmesg module.

This module makes extensive use of mock to avoid actually calling into dmesg,
which allows us to test all classes on all platforms, including windows.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import re
import warnings

try:
    from unittest import mock
except ImportError:
    import mock

import nose.tools as nt
import six

from . import utils
from framework import dmesg, status, results, exceptions

# pylint: disable=invalid-name,line-too-long,attribute-defined-outside-init


@nt.nottest
class TestDmesg(dmesg.BaseDmesg):
    """Test Dmesg class. stubs update_dmesg and __init__"""
    def update_dmesg(self, *args, **kwargs):
        pass

    def __init__(self):
        super(TestDmesg, self).__init__()
        self._new_messages = ['some', 'new', 'messages']


class TestBaseDmesg(object):
    """Tests for the BaseDmesg class."""
    @classmethod
    def setup_class(cls):
        cls.dmesg = TestDmesg()

    def setup(self):
        self.result = results.TestResult()
        self.result.dmesg = mock.sentinel.dmesg

    def test_update_result_dmesg(self):
        """dmesg.BaseDmesg.update_result: records new dmesg content in result"""
        self.dmesg.update_result(self.result)
        nt.assert_is_not(self.result.dmesg, mock.sentinel.dmesg)

    def test_update_result_status_unchanged(self):
        """dmesg.BaseDmesg.update_result: Doesn't change status it shouldn't

        Only 'pass', 'warn', and 'fail' should be changed.

        """
        failed = set()

        for stat in status.ALL:
            if stat in ['pass', 'warn', 'fail']:
                continue
            self.result.result = stat
            self.dmesg.update_result(self.result)
            if self.result.result != stat:
                failed.add(stat)

        if failed:
            raise AssertionError(
                "The following status(es) were changed which should not have "
                "been:\n"
                "{}\n".format('\n'.join(failed)))

    def test_update_result_status_changed(self):
        """dmesg.BaseDmesg.update_result: changes pass fail and warn"""
        failed = set()

        for stat in ['pass', 'warn', 'fail']:
            self.result.result = stat
            self.dmesg.update_result(self.result)
            if self.result.result == stat:
                failed.add(stat)

        if failed:
            raise AssertionError(
                "The following status(es) were not changed which should not "
                "have been:\n"
                "{}\n".format('\n'.join(failed)))

    def test_update_result_subtest_unchanged(self):
        """dmesg.BaseDmesg.update_result: Doesn't change subtests it shouldn't

        Only 'pass', 'warn', and 'fail' should be changed.

        """
        failed = set()

        for stat in status.ALL:
            if stat in ['pass', 'warn', 'fail']:
                continue
            self.result.subtests['foo'] = stat
            self.dmesg.update_result(self.result)
            if self.result.subtests['foo'] != stat:
                failed.add(stat)

        if failed:
            raise AssertionError(
                "The following status(es) were changed which should not have "
                "been:\n"
                "{}\n".format('\n'.join(failed)))

    def test_update_subtest_changed(self):
        """dmesg.BaseDmesg.update_result: changes subtests pass fail and warn"""
        failed = set()

        for stat in status.ALL:
            if stat in ['pass', 'warn', 'fail']:
                continue
            self.result.subtests['foo'] = stat
            self.dmesg.update_result(self.result)
            if self.result.subtests['foo'] != stat:
                failed.add(stat)

        if failed:
            raise AssertionError(
                "The following status(es) were changed which should not have "
                "been:\n"
                "{}\n".format('\n'.join(failed)))


def test_update_result_regex_no_match():
    """dmesg.BaseDmesg.update_result: if no regex matches don't change status"""
    dmesg_ = TestDmesg()
    dmesg_.regex = re.compile(r'nomatchforthisreally')
    result = results.TestResult('pass')
    dmesg_.update_result(result)

    nt.eq_(result.result, 'pass')


def test_update_result_regex_match():
    """dmesg.BaseDmesg.update_result: if regex matches change status"""
    dmesg_ = TestDmesg()
    dmesg_.regex = re.compile(r'.*')
    result = results.TestResult('pass')
    dmesg_.update_result(result)

    nt.assert_not_equal(result.result, 'pass')


@utils.nose.generator
def test_update_result_specific():
    """Generator that tests specific result mappings."""
    dmesg_ = TestDmesg()
    tests = [
        ('pass', 'dmesg-warn'),
        ('warn', 'dmesg-fail'),
        ('fail', 'dmesg-fail'),
    ]
    description = 'dmesg.BaseDmesg.update_result: replaces {} with {}'

    def test(initial, expected):
        result = results.TestResult(initial)
        dmesg_.update_result(result)
        nt.eq_(result.result, expected)

    for initial, expected in tests:
        test.description = description.format(initial, expected)
        yield test, initial, expected


@utils.nose.generator
def test_linuxdmesg_gzip_errors():
    """Generator to test exceptions that need to be passed when reading
    config.gz.

    """
    exceptions_ = {
        OSError,
        IOError,
    }
    description = "dmesg.LinuxDmesg: Doesn't stop on {} when reading gzip."

    @mock.patch('framework.dmesg.LinuxDmesg.update_dmesg', mock.Mock())
    def test(exception):
        try:
            with mock.patch('framework.dmesg.gzip.open',
                            mock.Mock(side_effect=exception)):
                with warnings.catch_warnings():
                    warnings.simplefilter('error')
                    dmesg.LinuxDmesg()
        except (exceptions.PiglitFatalError, RuntimeWarning):
            pass

    for exception in exceptions_:
        test.description = description.format(exception.__name__)
        yield test, exception


@nt.raises(exceptions.PiglitFatalError)
@mock.patch('framework.dmesg.gzip.open', mock.Mock(side_effect=IOError))
def test_linuxdmesg_timestamp():
    """dmesg.LinuxDmesg: If timestamps are not detected raise"""
    with mock.patch('framework.dmesg.subprocess.check_output',
                    mock.Mock(return_value=b'foo\nbar\n')):
        with warnings.catch_warnings():
            warnings.simplefilter('error')
            dmesg.LinuxDmesg()


@nt.raises(RuntimeWarning)
@mock.patch('framework.dmesg.gzip.open', mock.Mock(side_effect=IOError))
def test_linuxdmesg_warn():
    """dmesg.LinuxDmesg: Warn if timestamp support is uncheckable"""
    with mock.patch('framework.dmesg.LinuxDmesg.update_dmesg', mock.Mock()):
        with warnings.catch_warnings():
            warnings.simplefilter('error')
            dmesg.LinuxDmesg()


def test_linuxdmesg_update_dmesg_update():
    """dmesg.LinuxDmesg.update_dmesg: calculate dmesg changes correctly with changes"""
    result = results.TestResult('pass')

    with mock.patch('framework.dmesg.subprocess.check_output',
                    mock.Mock(return_value=b'[1.0]this')):
        dmesg_ = dmesg.LinuxDmesg()

    with mock.patch('framework.dmesg.subprocess.check_output',
                    mock.Mock(return_value=b'[1.0]this\n[2.0]is\n[3.0]dmesg\n')):
        dmesg_.update_result(result)

    nt.eq_(result.dmesg, '[2.0]is\n[3.0]dmesg')


def test_linuxdmesg_update_dmesg_update_no_change():
    """dmesg.LinuxDmesg.update_dmesg: calculate dmesg changes correctly with no changes"""
    result = results.TestResult('pass')
    result.dmesg = mock.sentinel.dmesg

    with mock.patch('framework.dmesg.subprocess.check_output',
                    mock.Mock(return_value=b'[1.0]this')):
        dmesg_ = dmesg.LinuxDmesg()
        dmesg_.update_result(result)

    nt.eq_(result.dmesg, mock.sentinel.dmesg)


def test_dummydmesg_update_result():
    """dmesg.DummyDmesg.update_result: returns result unmodified"""
    dmesg_ = dmesg.DummyDmesg()
    result = mock.MagicMock(spec=results.TestResult())
    result.dmesg = mock.sentinel.dmesg
    result.result = mock.sentinel.result
    dmesg_.update_result(result)

    nt.eq_(result.dmesg, mock.sentinel.dmesg)
    nt.eq_(result.result, mock.sentinel.result)


@utils.nose.generator
def test_get_dmesg():
    """Generate tests for get_dmesg."""
    tests = [
        ('linux', dmesg.LinuxDmesg),
        # There is no dmesg on windows, thus it will always get the dummy
        ('win32', dmesg.DummyDmesg),
    ]
    description = 'dmesg.get_dmesg: returns correct class when platform is {}'

    def test(platform, class_):
        with mock.patch('framework.dmesg.sys.platform', platform):
            ret = dmesg.get_dmesg()
        nt.assert_is_instance(ret, class_)

    for platform, class_ in tests:
        test.description = description.format(platform)
        yield test, platform, class_


def test_get_dmesg_dummy():
    """dmesg.get_dmesg: when not_dummy=False a dummy is provided"""
    # Linux was selected since it would normally return LinuxDmesg
    with mock.patch('framework.dmesg.sys.platform', 'linux'):
        ret = dmesg.get_dmesg(False)
    nt.assert_is_instance(ret, dmesg.DummyDmesg)


def test_partial_wrap():
    """dmesg.LinuxDmesg.update_dmesg: correctly handles partial wrap

    Since dmesg is a ringbuffer (at least on Linux) it can roll over, and we
    need to ensure that we're handling that correctly.

    """
    result = results.TestResult()

    mock_out = mock.Mock(return_value=b'[1.0]This\n[2.0]is\n[3.0]dmesg')
    with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
        test = dmesg.LinuxDmesg()

    mock_out.return_value = b'[3.0]dmesg\n[4.0]whoo!'
    with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
        test.update_result(result)

    nt.eq_(result.dmesg, '[4.0]whoo!')


def test_complete_wrap():
    """dmesg.LinuxDmesg.update_dmesg: correctly handles complete wrap

    Since dmesg is a ringbuffer (at least on Linux) it can roll over, and we
    need to ensure that we're handling that correctly.

    """
    result = results.TestResult()

    mock_out = mock.Mock(return_value=b'[1.0]This\n[2.0]is\n[3.0]dmesg')
    with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
        test = dmesg.LinuxDmesg()

    mock_out.return_value = b'[4.0]whoo!\n[5.0]doggy'
    with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
        test.update_result(result)

    nt.eq_(result.dmesg, '[4.0]whoo!\n[5.0]doggy')
