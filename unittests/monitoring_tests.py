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

try:
    from unittest import mock
except ImportError:
    import mock

import nose.tools as nt

from . import utils
from framework import monitoring, exceptions


class TestMonitoring(object):
    """Tests for Monitoring methods."""

    def __init__(self):
        """Setup for TestMonitoring

        This create a monitoring.Monitoring instance with monitoring disabled
        to avoid reading the rules in piglit.conf.

        """
        self.regex = r'\*ERROR\*|BUG:'
        self.init_contents = r'foo bar\n'
        self.no_error_contents = r'foo bar\n'
        self.error_contents = r'BUG:bar\n'
        self.monitoring = monitoring.Monitoring(False)

    def test_Monitoring_delete_rule(self):
        """monitorin.Monitoring: add and delete rule."""

        with utils.nose.tempfile(self.init_contents) as tfile:
            self.monitoring.add_rule('error_file',
                                     'file',
                                     tfile,
                                     self.regex)
            self.monitoring.update_monitoring()

        self.monitoring.delete_rule('error_file')
        with open(tfile, 'w') as fp:
            fp.write(self.error_contents)
            fp.close()
            self.monitoring.check_monitoring()

        nt.assert_equal(self.monitoring.abort_needed, False)

    @nt.raises(exceptions.PiglitFatalError)
    def test_Monitoring_add_rule_bad_format(self):
        """monitoring.Monitoring: add non existing type rule."""

        with utils.nose.tempfile(self.init_contents) as tfile:
            self.monitoring.add_rule('error_file_bad_type',
                                     'bad_type',
                                     tfile,
                                     self.regex)

    def test_Monitoring_file_error(self):
        """monitoring.Monitoring: error found on a file."""

        with utils.nose.tempfile(self.init_contents) as tfile:
            self.monitoring.add_rule('error_file',
                                     'file',
                                     tfile,
                                     self.regex)
            self.monitoring.update_monitoring()

        with open(tfile, 'w') as fp:
            fp.write(self.error_contents)
            fp.close()
            self.monitoring.check_monitoring()

        nt.assert_equal(self.monitoring.abort_needed, True)

    def test_Monitoring_file_no_error(self):
        """monitoring.Monitoring: no error found on a file."""

        with utils.nose.tempfile(self.init_contents) as tfile:
            self.monitoring.add_rule('no_error_file',
                                     'file',
                                     tfile,
                                     self.regex)
            self.monitoring.update_monitoring()

        with open(tfile, 'w') as fp:
            fp.write(self.no_error_contents)
            fp.close()
            self.monitoring.check_monitoring()

        nt.assert_equal(self.monitoring.abort_needed, False)

    def test_Monitoring_locked_file_error(self):
        """monitoring.Monitoring: error found on a locked file."""

        with utils.nose.tempfile(self.init_contents) as tfile:
            self.monitoring.add_rule('error_locked_file',
                                     'locked_file',
                                     tfile,
                                     self.regex)
            self.monitoring.update_monitoring()

        with open(tfile, 'w') as fp:
            fp.write(self.error_contents)
            fp.close()
            self.monitoring.check_monitoring()

        nt.assert_equal(self.monitoring.abort_needed, True)

    def test_Monitoring_locked_file_no_error(self):
        """monitoring.Monitoring: no error found on a locked file."""

        with utils.nose.tempfile(self.init_contents) as tfile:
            self.monitoring.add_rule('no_error_file',
                                     'locked_file',
                                     tfile,
                                     self.regex)
            self.monitoring.update_monitoring()

        with open(tfile, 'w') as fp:
            fp.write(self.no_error_contents)
            fp.close()
            self.monitoring.check_monitoring()

        nt.assert_equal(self.monitoring.abort_needed, False)

    def test_Monitoring_dmesg_error(self):
        """monitoring.Monitoring: error found on the dmesg."""

        utils.nose.platform_check('linux')

        mock_out = mock.Mock(return_value=b'[1.0]This\n[2.0]is\n[3.0]dmesg')
        with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
            self.monitoring.add_rule('no_error_file',
                                 'dmesg',
                                 '--level emerg,alert,crit,err',
                                 self.regex)
            self.monitoring.update_monitoring()

        mock_out.return_value = b'[4.0]foo\n[5.0]*ERROR* bar'
        with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
            self.monitoring.check_monitoring()

        nt.assert_equal(self.monitoring.abort_needed, True)

    def test_Monitoring_dmesg_no_error(self):
        """monitoring.Monitoring: no error found on the dmesg."""

        utils.nose.platform_check('linux')

        mock_out = mock.Mock(return_value=b'[1.0]This\n[2.0]is\n[3.0]dmesg')
        with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
            self.monitoring.add_rule('no_error_file',
                                 'dmesg',
                                 '--level emerg,alert,crit,err',
                                 self.regex)
            self.monitoring.update_monitoring()

        mock_out.return_value = b'[4.0]foo\n[5.0] bar'
        with mock.patch('framework.dmesg.subprocess.check_output', mock_out):
            self.monitoring.check_monitoring()

        nt.assert_equal(self.monitoring.abort_needed, False)
