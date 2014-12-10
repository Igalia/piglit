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

"""Tests for the json outpt format.

This module drills the ouput of the json backend with a series of tests
designed to catch changes in the json output. This is a rather volatile set of
tests and they will change with each version of the json output.

"""

from __future__ import print_function, absolute_import
import os

import nose.tools as nt
try:
    import simplejson as json
except ImportError:
    import json

import framework.core as core
import framework.tests.utils as utils
from framework.backends.json_ import JSONBackend
from framework.programs.run import _create_metadata


# Helpers
class Namespace(object):
    """Simple namespace object for replicating and argparse namespace."""
    pass


# Tests
# pylint: disable=too-many-public-methods
class TestJsonOutput(utils.StaticDirectory):
    """Class for testing JSON output."""
    @classmethod
    def setup_class(cls):
        super(TestJsonOutput, cls).setup_class()

        args = Namespace()
        # pylint: disable=attribute-defined-outside-init
        args.test_profile = ['fake.py']
        args.platform = 'gbm'
        args.log_level = 'verbose'

        backend = JSONBackend(cls.tdir, file_fsync=True)
        backend.initialize(_create_metadata(args, 'test', core.Options()))
        backend.write_test('result', {'result': 'pass'})
        backend.finalize({'time_elapsed': 1.22})
        with open(os.path.join(cls.tdir, 'results.json'), 'r') as f:
            cls.json = json.load(f)

    def test_root_results_version(self):
        """JSON: result_version is a root key."""
        nt.assert_in('results_version', self.json)

    def test_root_name(self):
        """JSON: name is a root key."""
        nt.assert_in('name', self.json)

    def test_root_options(self):
        """JSON: options is a root key."""
        nt.assert_in('options', self.json)

    def test_root_tests(self):
        """JSON: tests is a root key."""
        nt.assert_in('tests', self.json)

    def test_root_lspci(self):
        """JSON: lspci is a root key."""
        utils.platform_check('linux')
        utils.binary_check('lspci')
        nt.assert_in('lspci', self.json)

    def test_root_uname(self):
        """JSON: uname is a root key."""
        utils.platform_check('linux')
        utils.binary_check('uname')
        nt.assert_in('uname', self.json)

    def test_root_glxinfo(self):
        """JSON: glxinfo is a root key."""
        utils.platform_check('linux')
        utils.binary_check('glxinfo')
        nt.assert_in('glxinfo', self.json)

    def test_root_time_elapsed(self):
        """JSON: time_elapsed is a root key."""
        nt.assert_in('time_elapsed', self.json)

    def test_options_profile(self):
        """JSON: profile is an options key."""
        nt.assert_in('profile', self.json['options'])

    def test_options_dmesg(self):
        """JSON: dmesg is an options key."""
        nt.assert_in('dmesg', self.json['options'])

    def test_options_execute(self):
        """JSON: execute is an options key."""
        nt.assert_in('execute', self.json['options'])

    def test_options_log_level(self):
        """JSON: log_level is an options key."""
        nt.assert_in('log_level', self.json['options'])

    def test_options_platform(self):
        """JSON: platform is an options key."""
        nt.assert_in('platform', self.json['options'])

    def test_options_sync(self):
        """JSON: sync is an options key."""
        nt.assert_in('sync', self.json['options'])

    def test_options_valgrind(self):
        """JSON: valgrind is an options key."""
        nt.assert_in('valgrind', self.json['options'])

    def test_options_concurrent(self):
        """JSON: concurrent is an options key."""
        nt.assert_in('concurrent', self.json['options'])

    def test_options_filter(self):
        """JSON: filter is an options key."""
        nt.assert_in('filter', self.json['options'])

    def test_options_exclude_tests(self):
        """JSON: exclude_tests is an options key."""
        nt.assert_in('exclude_tests', self.json['options'])

    def test_options_exclude_filter(self):
        """JSON: exclude_filter is an options key."""
        nt.assert_in('exclude_filter', self.json['options'])
