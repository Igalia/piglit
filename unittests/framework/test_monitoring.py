# coding=utf-8
# Copyright (c) 2016 Intel Corporation

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

"""Tests for the monitoring module.

This provides tests for the framework.monitoring modules.
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import pytest
import six

from framework import exceptions
from framework import monitoring
from . import skip

# pylint: disable=no-self-use,attribute-defined-outside-init


class TestMonitoring(object):
    """Tests for Monitoring methods."""

    def setup(self):
        """Setup for TestMonitoring.

        This create a monitoring.Monitoring instance with monitoring disabled
        to avoid reading the rules in piglit.conf.
        """
        self.regex = r'\*ERROR\*|BUG:'
        self.init_contents = r'foo bar\n'
        self.no_error_contents = r'foo bar\n'
        self.error_contents = r'BUG:bar\n'
        self.monitoring = monitoring.Monitoring(False)

    @skip.linux
    def test_delete_rule(self, tmpdir):
        """monitoring.Monitoring: add and delete rule."""
        p = tmpdir.join('foo')
        p.write(self.init_contents)
        self.monitoring.add_rule('error_file',
                                 'file',
                                 six.text_type(p),
                                 self.regex)
        self.monitoring.update_monitoring()

        self.monitoring.delete_rule('error_file')
        p.write(self.error_contents)
        self.monitoring.check_monitoring()

        assert self.monitoring.abort_needed is False

    def test_add_rule_bad_format(self, tmpdir):
        """monitoring.Monitoring: add non existing type rule."""
        p = tmpdir.join('foo')

        with pytest.raises(exceptions.PiglitFatalError):
            self.monitoring.add_rule('error_file_bad_type',
                                     'bad_type',
                                     six.text_type(p),
                                     self.regex)

    @skip.linux
    def test_file_error(self, tmpdir):
        """monitoring.Monitoring: error found on a file."""
        p = tmpdir.join('foo')
        p.write(self.init_contents)
        self.monitoring.add_rule('error_file',
                                 'file',
                                 six.text_type(p),
                                 self.regex)
        self.monitoring.update_monitoring()

        p.write(self.error_contents)
        self.monitoring.check_monitoring()

        assert self.monitoring.abort_needed is True

    @skip.linux
    def test_file_no_error(self, tmpdir):
        """monitoring.Monitoring: no error found on a file."""
        p = tmpdir.join('foo')
        p.write(self.init_contents)
        self.monitoring.add_rule('no_error_file',
                                 'file',
                                 six.text_type(p),
                                 self.regex)
        self.monitoring.update_monitoring()

        p.write(self.no_error_contents)
        self.monitoring.check_monitoring()

        assert self.monitoring.abort_needed is False

    @skip.linux
    def test_locked_file_error(self, tmpdir):
        """monitoring.Monitoring: error found on a locked file."""
        p = tmpdir.join('foo')
        p.write(self.init_contents)
        self.monitoring.add_rule('error_locked_file',
                                 'locked_file',
                                 six.text_type(p),
                                 self.regex)
        self.monitoring.update_monitoring()

        p.write(self.error_contents)
        self.monitoring.check_monitoring()

        assert self.monitoring.abort_needed is True

    @skip.linux
    def test_locked_file_no_error(self, tmpdir):
        """monitoring.Monitoring: no error found on a locked file."""
        p = tmpdir.join('foo')
        p.write(self.init_contents)
        self.monitoring.add_rule('no_error_file',
                                 'locked_file',
                                 six.text_type(p),
                                 self.regex)
        self.monitoring.update_monitoring()

        p.write(self.no_error_contents)
        self.monitoring.check_monitoring()

        assert self.monitoring.abort_needed is False

    @skip.linux
    def test_dmesg_error(self, mocker):
        """monitoring.Monitoring: error found on the dmesg."""
        mocker.patch('framework.dmesg.subprocess.check_output',
                     mocker.Mock(return_value=b'[1.0]This\n[2.0]is\n[3.0]dmesg'))
        self.monitoring.add_rule('no_error_file',
                                 'dmesg',
                                 '--level emerg,alert,crit,err',
                                 self.regex)
        self.monitoring.update_monitoring()

        mocker.patch('framework.dmesg.subprocess.check_output',
                     mocker.Mock(return_value=b'[4.0]foo\n[5.0]*ERROR* bar'))
        self.monitoring.check_monitoring()

        assert self.monitoring.abort_needed is True

    @skip.linux
    def test_dmesg_no_error(self, mocker):
        """monitoring.Monitoring: no error found on the dmesg."""
        mocker.patch('framework.dmesg.subprocess.check_output',
                     mocker.Mock(return_value=b'[1.0]This\n[2.0]is\n[3.0]dmesg'))
        self.monitoring.add_rule('no_error_file',
                                 'dmesg',
                                 '--level emerg,alert,crit,err',
                                 self.regex)
        self.monitoring.update_monitoring()

        mocker.patch('framework.dmesg.subprocess.check_output',
                     mocker.Mock(return_value=b'[4.0]foo\n[5.0] bar'))
        self.monitoring.check_monitoring()

        assert self.monitoring.abort_needed is False
