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
import sys
import subprocess

import framework.grouptools as grouptools
import framework.core
from framework.profile import TestProfile, Test

__all__ = ['profile']


def check_environment():
    """Check that the environment that piglit is running in is appropriate.

    IGT requires root, debugfs to be mounted, and to be the only drm client.

    """
    debugfs_path = "/sys/kernel/debug/dri"
    if os.getuid() != 0:
        print("Test Environment check: not root!")
        return False
    if not os.path.isdir(debugfs_path):
        print("Test Environment check: debugfs not mounted properly!")
        return False
    for subdir in os.listdir(debugfs_path):
        if not os.path.isdir(os.path.join(debugfs_path, subdir)):
            continue
        clients = open(os.path.join(debugfs_path, subdir, "clients"), 'r')
        lines = clients.readlines()
        if len(lines) > 2:
            print("Test Environment check: other drm clients running!")
            return False

    print("Test Environment check: Succeeded.")
    return True


if 'IGT_TEST_ROOT' in os.environ:
    IGT_TEST_ROOT = os.environ['IGT_TEST_ROOT']
else:
    IGT_TEST_ROOT = os.path.join(
        framework.core.PIGLIT_CONFIG.get('igt', 'path'), 'tests')
    assert os.path.exists(IGT_TEST_ROOT)

# check for the test lists
if not (os.path.exists(os.path.join(IGT_TEST_ROOT, 'single-tests.txt'))
        and os.path.exists(os.path.join(IGT_TEST_ROOT, 'multi-tests.txt'))):
    print("intel-gpu-tools test lists not found.")
    sys.exit(0)


class IGTTestProfile(TestProfile):
    """Test profile for intel-gpu-tools tests."""
    def _pre_run_hook(self):
        if not check_environment():
            sys.exit(1)


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
    with open(os.path.join(IGT_TEST_ROOT, listname + '.txt'), 'r') as f:
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
    proc = subprocess.Popen(
        [os.path.join(IGT_TEST_ROOT, test), '--list-subtests'],
        stdout=subprocess.PIPE,
        env=os.environ.copy(),
        universal_newlines=True)
    out, _ = proc.communicate()

    # a return code of 79 indicates there are no subtests
    if proc.returncode == 79:
        profile.test_list[grouptools.join('igt', test)] = IGTTest(test)
        return

    if proc.returncode != 0:
        print("Error: Could not list subtests for " + test)
        return

    subtests = out.split("\n")

    for subtest in subtests:
        if subtest == "":
            continue
        profile.test_list[grouptools.join('igt', test, subtest)] = \
            IGTTest(test, ['--run-subtest', subtest])


def populate_profile():
    tests = list_tests("single-tests")
    tests.extend(list_tests("multi-tests"))

    for test in tests:
        add_subtest_cases(test)


populate_profile()
profile.dmesg = True

# the dmesg property of TestProfile returns a Dmesg object
profile.dmesg.regex = re.compile(r"(\[drm:|drm_|intel_|i915_)")
