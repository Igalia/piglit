# coding=utf-8
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

"""Tests for the dmesg module.

This module makes extensive use of mock to avoid actually calling into dmesg,
which allows us to test all classes on all platforms, including windows.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import re
try:
    import mock
except ImportError:
    from unittest import mock

import pytest
import six

from framework import dmesg
from framework import status
from framework import results
from framework import exceptions

from . import skip

# pylint: disable=invalid-name,no-self-use


class _DmesgTester(dmesg.BaseDmesg):
    """Test Dmesg class. stubs update_dmesg and __init__"""

    def __init__(self):
        super(_DmesgTester, self).__init__()
        self._new_messages = ['some', 'new', 'messages']

    def update_dmesg(self, *args, **kwargs):
        pass


class TestBaseDmesg(object):
    """Tests for the BaseDmesg class."""

    result = None

    @classmethod
    def setup_class(cls):
        cls.dmesg = _DmesgTester()

    def setup(self):
        self.result = results.TestResult()
        self.result.dmesg = mock.sentinel.dmesg

    def test_update_result_dmesg(self):
        """dmesg.BaseDmesg.update_result: records new dmesg content in result"""
        self.dmesg.update_result(self.result)
        assert self.result.dmesg is not mock.sentinel.dmesg

    @pytest.mark.parametrize(
        "initial,expected",
        [
            (status.PASS, status.DMESG_WARN),
            (status.WARN, status.DMESG_FAIL),
            (status.FAIL, status.DMESG_FAIL),
            (status.CRASH, status.CRASH),
            (status.SKIP, status.SKIP),
            (status.NOTRUN, status.NOTRUN),
            (status.TIMEOUT, status.TIMEOUT),
        ],
        ids=six.text_type)
    def test_update_result_status(self, initial, expected):
        """Test that when update_result is called status change when they
        should, and don't when they shouldn't.
        """
        self.result.result = initial
        self.dmesg.update_result(self.result)
        assert self.result.result is expected

    @pytest.mark.parametrize(
        "initial,expected",
        [
            (status.PASS, status.DMESG_WARN),
            (status.WARN, status.DMESG_FAIL),
            (status.FAIL, status.DMESG_FAIL),
            (status.CRASH, status.CRASH),
            (status.SKIP, status.SKIP),
            (status.NOTRUN, status.NOTRUN),
            (status.TIMEOUT, status.TIMEOUT),
        ],
        ids=six.text_type)
    def test_update_result_subtests(self, initial, expected):
        """Test that when update_result is called subtest statuses change when
        they should, and don't when they shouldn't.
        """
        self.result.subtests['foo'] = initial
        self.result.subtests['bar'] = initial

        self.dmesg.update_result(self.result)

        assert self.result.subtests['foo'] is expected
        assert self.result.subtests['bar'] is expected

    def test_update_result_regex_no_match(self):
        """dmesg.BaseDmesg.update_result: if no regex matches don't change
        status.
        """
        self.dmesg.regex = re.compile(r'nomatchforthisreally')
        self.result.result = status.PASS

        self.dmesg.update_result(self.result)

        assert self.result.result is status.PASS

    def test_update_result_regex_match(self):
        """dmesg.BaseDmesg.update_result: if regex matches change status."""
        self.dmesg.regex = re.compile(r'.*')
        self.result.result = status.PASS

        self.dmesg.update_result(self.result)

        assert self.result.result is status.DMESG_WARN


class TestLinuxDmesgTimestamps(object):
    """Tests for the LinuxDmesg.__init__ timestampe detection.

    The linux path will try to open /proc/config.gz and look for
    CONFIG_PRINTK_TIME=y, there are a couple of things that can go wrong it
    should recover from, in both of the following cases it's expected to go
    on and fall back to less exact checking.

    The first is that there is no /proc/config.gz in that case we'll get a
    OSError in python2 or it's descendent FileNotFoundError in python 3.

    The second is that it could get an IOError in python 2.x or
    PermissionError in python 3.x, if the file cannot be read.

    The fallback path will try read dmesg and see if there's evidence of a
    timestamp, or warn that it can't find anything.

    """
    def test_warn(self, mocker):
        """Test that if /proc/config is not available, and there are no values
        to check in _last_message a RuntimeWarning should be issued.
        """
        # Mocking this will prevent LinuxDmesg._last_message from being
        # updated, which will force us down the warn path
        mocker.patch('framework.dmesg.LinuxDmesg.update_dmesg')

        # OSError was picked because it will work for both python2 and python3
        # (FileNotFoundError is a descendent of OSError)
        mocker.patch('framework.dmesg.gzip.open', side_effect=OSError)

        with pytest.warns(RuntimeWarning):
            dmesg.LinuxDmesg()

    # Implementation notes:
    #
    # It would seem like a parametrized test would be what we want here, since
    # these tests share so much state, but FileNotFoundError and
    # PermissionError don't exist in python 2.x, and the parametrizer will
    # cause an error. Using a class with a shared method is the next best
    # solution.
    @staticmethod
    def _do_test(error, mocker):
        # Mocking this will prevent LinuxDmesg._last_message from being
        # updated, which will force us down the warn path, which is convenient
        # for assertion puproses.
        mocker.patch('framework.dmesg.LinuxDmesg.update_dmesg')
        mocker.patch('framework.dmesg.gzip.open', side_effect=error)

        with pytest.warns(RuntimeWarning):
            dmesg.LinuxDmesg()

    @skip.PY3
    def test_config_oserror(self, mocker):
        """Test that on python 2.x if an OSError is raised by gzip.open
        operation doesn't stop.
        """
        self._do_test(OSError, mocker)

    @skip.PY3
    def test_config_ioerror(self, mocker):
        """Test that on python 2.x if an IOError is raised by gzip.open
        operation doesn't stop.
        """
        self._do_test(IOError, mocker)

    @skip.PY2
    def test_config_filenotfounderror(self, mocker):
        """Test that on python 3.x if an FileNotFound is raised by gzip.open
        operation doesn't stop.
        """
        self._do_test(FileNotFoundError, mocker)

    @skip.PY2
    def test_config_permissionerror(self, mocker):
        """Test that on python 3.x if an PermissionError is raised by gzip.open
        operation doesn't stop.
        """
        self._do_test(PermissionError, mocker)

    def test_not_timestamps(self, mocker):
        """If _last_message is populated but doesn't have a valid timestamp
        then an PiglitFatalException shoudl be raised.
        """
        mocker.patch('framework.dmesg.subprocess.check_output',
                     mocker.Mock(return_value=b'foo\nbar\n'))
        # This error will work for python 2.x and 3.x
        mocker.patch('framework.dmesg.gzip.open', side_effect=OSError)

        with pytest.raises(exceptions.PiglitFatalError):
            dmesg.LinuxDmesg()

    @skip.linux
    def test_partial_wrap(self):
        """dmesg.LinuxDmesg.update_dmesg: correctly handles partial wrap.

        Since dmesg is a ringbuffer it can roll over, and we need to ensure
        that we're handling that correctly.
        """
        result = results.TestResult()

        mock_out = mock.Mock(return_value=b'[1.0]This\n[2.0]is\n[3.0]dmesg')
        with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
            test = dmesg.LinuxDmesg()

        mock_out.return_value = b'[3.0]dmesg\n[4.0]whoo!'
        with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
            test.update_result(result)

        assert result.dmesg == '[4.0]whoo!'

    @skip.linux
    def test_complete_wrap(self):
        """dmesg.LinuxDmesg.update_dmesg: correctly handles complete wrap.

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

        assert result.dmesg == '[4.0]whoo!\n[5.0]doggy'


class TestLinuxDmesg(object):
    """Tests for LinuxDmesg methods."""

    def test_update_result_sets_result_attr(self):
        """When update_dmesg is called on a value it should set the dmesg
        attribute of the results.
        """
        result = results.TestResult(status.PASS)

        with mock.patch('framework.dmesg.subprocess.check_output',
                        mock.Mock(return_value=b'[1.0]this')):
            test = dmesg.LinuxDmesg()
        with mock.patch('framework.dmesg.subprocess.check_output',
                        mock.Mock(
                            return_value=b'[1.0]this\n[2.0]is\n[2.5]dmesg!\n')):
            test.update_result(result)

        assert result.dmesg == '[2.0]is\n[2.5]dmesg!'

    def test_update_result_no_change(self):
        """When update_result is called but no changes to dmesg have occured it
        should not set the dmesg attribute.
        """
        result = results.TestResult('pass')
        result.dmesg = mock.sentinel.dmesg

        with mock.patch('framework.dmesg.subprocess.check_output',
                        mock.Mock(return_value=b'[1.0]this')):
            test = dmesg.LinuxDmesg()
            test.update_result(result)

        assert result.dmesg is mock.sentinel.dmesg

    def test_repr(self):
        with mock.patch('framework.dmesg.subprocess.check_output',
                        mock.Mock(return_value=b'[1.0]this')):
            assert repr(dmesg.LinuxDmesg()) == 'LinuxDmesg()'


class TestDummyDmesg(object):
    """Tests for the DummyDmesg class."""
    _Namespace = collections.namedtuple('_Namespace', ['dmesg', 'result'])

    @pytest.fixture
    def testers(self):
        # The setter for result.TestResult checks types, rather than trying to
        # make the sentinel pass that test, just make a mock that mostly acts
        # like TestResult
        result = mock.Mock(spec=results.TestResult)
        result.dmesg = mock.sentinel.dmesg
        result.result = mock.sentinel.result
        return self._Namespace(dmesg.DummyDmesg(), result)

    def test_update_result(self, testers):
        """DummyDmesg.update_results shouldn't do anything."""
        testers.dmesg.update_result(testers.result)
        assert testers.result.dmesg is mock.sentinel.dmesg
        assert testers.result.result is mock.sentinel.result

    def test_repr(self):
        assert repr(dmesg.DummyDmesg()) == 'DummyDmesg()'


def _name_get_dmesg(value):
    """Function that names TestGetDmesg.test_get_dmesg."""
    if isinstance(value, bool):
        return 'real' if not value else 'dummy'
    elif isinstance(value, six.text_type):
        return value
    elif isinstance(value, dmesg.BaseDmesg):
        return repr(value)
    else:
        raise Exception('unreachable')


class TestGetDmesg(object):
    """Tests for get_dmesg factory."""

    @pytest.mark.parametrize(
        'platform,dummy,expected',
        [
            ('win32', False, dmesg.DummyDmesg),
            ('win32', True, dmesg.DummyDmesg),
            skip.linux(('linux', False, dmesg.DummyDmesg)),
            skip.linux(('linux', True, dmesg.LinuxDmesg)),
        ],
        ids=_name_get_dmesg)
    def test_get_dmesg(self, platform, dummy, expected, mocker):
        """Test that get_dmesg returns the expected dmesg type on variuos
        platforms with various configurations.
        """
        mocker.patch('framework.dmesg.sys.platform', platform)

        with mock.patch('framework.dmesg.subprocess.check_output',
                        mock.Mock(return_value=b'[1.0]foo')):
            actual = dmesg.get_dmesg(not_dummy=dummy)

        # We don't want a subclass, we want the *exact* class. This is a
        # unittest after all
        assert type(actual) == expected  # pylint: disable=unidiomatic-typecheck
