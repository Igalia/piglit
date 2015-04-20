#
# Copyright (c) 2012 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

"""Integration for running intel-gpu-tools with the piglit framework.

To use this either configure piglit.conf's [igt] section, or set IGT_TEST_ROOT
to the root of a built igt directory.

This will stop if you are not running as root, or if there are other users of
drm. Even if you have rendernode support enabled.

"""

from __future__ import print_function, division, absolute_import
import os
import re
import subprocess

from framework import grouptools, exceptions, core
from framework.profile import TestProfile, Test

__all__ = ['profile']


def check_environment():
    """Check that the environment that piglit is running in is appropriate.

    IGT requires root, debugfs to be mounted, and to be the only drm client.

    """
    debugfs_path = "/sys/kernel/debug/dri"
    if os.getuid() != 0:
        raise exceptions.PiglitInternalError(
            "Test Environment check: not root!")
    if not os.path.isdir(debugfs_path):
        raise exceptions.PiglitInternalError(
            "Test Environment check: debugfs not mounted properly!")
    for subdir in os.listdir(debugfs_path):
        if not os.path.isdir(os.path.join(debugfs_path, subdir)):
            continue
        clients = open(os.path.join(debugfs_path, subdir, "clients"), 'r')
        lines = clients.readlines()
        if len(lines) > 2:
            raise exceptions.PiglitInternalError(
                "Test Environment check: other drm clients running!")


if 'IGT_TEST_ROOT' in os.environ:
    IGT_TEST_ROOT = os.environ['IGT_TEST_ROOT']
else:
    IGT_TEST_ROOT = os.path.join(
        core.PIGLIT_CONFIG.required_get('igt', 'path'), 'tests')

if not os.path.exists(IGT_TEST_ROOT):
    raise exceptions.PiglitFatalError(
        'IGT directory does not exist. Missing: {}'.format(IGT_TEST_ROOT))

# check for the test lists
if os.path.exists(os.path.join(IGT_TEST_ROOT, 'test-list.txt')):
    TEST_LISTS = ['test-list.txt']
elif (os.path.exists(os.path.join(IGT_TEST_ROOT, 'single-tests.txt')) and
      os.path.exists(os.path.join(IGT_TEST_ROOT, 'multi-tests.txt'))):
    TEST_LISTS = ['single-tests.txt', 'multi-tests.txt']
else:
    raise exceptions.PiglitFatalError("intel-gpu-tools test lists not found.")


class IGTTestProfile(TestProfile):
    """Test profile for intel-gpu-tools tests."""
    def _pre_run_hook(self, opts):
        if opts.execute:
            try:
                check_environment()
            except exceptions.PiglitInternalError as e:
                raise exceptions.PiglitFatalError(e.message)


profile = IGTTestProfile()  # pylint: disable=invalid-name


class IGTTest(Test):
    """Test class for running libdrm."""
    def __init__(self, binary, arguments=None):
        if arguments is None:
            arguments = []
        super(IGTTest, self).__init__(
            [os.path.join(IGT_TEST_ROOT, binary)] + arguments)
        self.timeout = 600

    def interpret_result(self):
        if self.result['returncode'] == 0:
            self.result['result'] = 'pass'
        elif self.result['returncode'] == 77:
            self.result['result'] = 'skip'
        elif self.result['returncode'] == 78:
            self.result['result'] = 'timeout'
        else:
            self.result['result'] = 'fail'


def list_tests(listname):
    """Parse igt test list and return them as a list."""
    with open(os.path.join(IGT_TEST_ROOT, listname), 'r') as f:
        lines = (line.rstrip() for line in f.readlines())

    found_header = False

    for line in lines:
        if found_header:
            return line.split(" ")

        if "TESTLIST" in line:
            found_header = True

    return []


def add_subtest_cases(test):
    """Get subtest instances."""
    try:
        out = subprocess.check_output(
            [os.path.join(IGT_TEST_ROOT, test), '--list-subtests'],
            env=os.environ.copy(),
            universal_newlines=True)
    except subprocess.CalledProcessError as e:
        # a return code of 79 indicates there are no subtests
        if e.returncode == 79:
            profile.test_list[grouptools.join('igt', test)] = IGTTest(test)
        elif e.returncode != 0:
            print("Error: Could not list subtests for " + test)
        else:
            raise

        # If we reach here there are no subtests.
        return

    for subtest in (s for s in out.splitlines() if s):
        profile.test_list[grouptools.join('igt', test, subtest)] = \
            IGTTest(test, ['--run-subtest', subtest])


def populate_profile():
    tests = []
    for test_list in TEST_LISTS:
        tests.extend(list_tests(test_list))

    for test in tests:
        add_subtest_cases(test)


populate_profile()
profile.dmesg = True

# the dmesg property of TestProfile returns a Dmesg object
profile.dmesg.regex = re.compile(r"(\[drm:|drm_|intel_|i915_)")
