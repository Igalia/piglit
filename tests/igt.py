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

import os
import re
import sys
import subprocess
import threading
import time
import signal
import errno
from datetime import datetime

from os import path
import framework.core
from framework.profile import TestProfile, Test

__all__ = ['profile']

#############################################################################
##### IGTTest: Execute an intel-gpu-tools test
#####
##### To use this, create an igt symlink in piglit/bin which points to the root
##### of the intel-gpu-tools sources with the compiled tests. Piglit will
##### automatically add all tests into the 'igt' category.
#############################################################################

def checkEnvironment():
    debugfs_path = "/sys/kernel/debug/dri"
    if os.getuid() != 0:
        print "Test Environment check: not root!"
        return False
    if not os.path.isdir(debugfs_path):
        print "Test Environment check: debugfs not mounted properly!"
        return False
    for subdir in os.listdir(debugfs_path):
        if not os.path.isdir(os.path.join(debugfs_path, subdir)):
            continue
        clients = open(os.path.join(debugfs_path, subdir, "clients"), 'r')
        lines = clients.readlines()
        if len(lines) > 2:
            print "Test Environment check: other drm clients running!"
            return False

    print "Test Environment check: Succeeded."
    return True

if 'IGT_TEST_ROOT' in os.environ:
    igtTestRoot = os.environ['IGT_TEST_ROOT']
else:
    igtTestRoot = os.path.join(framework.core.PIGLIT_CONFIG.get('igt', 'path'),
                               'tests')
    assert os.path.exists(igtTestRoot)

# check for the test lists
if not (os.path.exists(os.path.join(igtTestRoot, 'single-tests.txt'))
        and os.path.exists(os.path.join(igtTestRoot, 'multi-tests.txt'))):
    print "intel-gpu-tools test lists not found."
    sys.exit(0)

igtEnvironmentOk = checkEnvironment()

profile = TestProfile()

class IGTTest(Test):
    def __init__(self, binary, arguments=None):
        if arguments is None:
            arguments = []
        super(IGTTest, self).__init__(
            [path.join(igtTestRoot, binary)] + arguments)
        self.timeout = 600

    def interpret_result(self):
        if not igtEnvironmentOk:
            return

        if self.result['returncode'] == 0:
            self.result['result'] = 'pass'
        elif self.result['returncode'] == 77:
            self.result['result'] = 'skip'
        elif self.result['returncode'] == 78:
            self.result['result'] = 'timeout'
        else:
            self.result['result'] = 'fail'

    def run(self):
        if not igtEnvironmentOk:
            self.result['result'] = 'fail'
            self.result['info'] = unicode("Test Environment isn't OK")
            return

        super(IGTTest, self).run()

def listTests(listname):
    with open(path.join(igtTestRoot, listname + '.txt'), 'r') as f:
        lines = (line.rstrip() for line in f.readlines())

    found_header = False
    progs = ""

    for line in lines:
        if found_header:
            progs = line.split(" ")
            break

        if "TESTLIST" in line:
            found_header = True

    return progs

tests = listTests("single-tests")
tests.extend(listTests("multi-tests"))

def addSubTestCases(test):
    proc = subprocess.Popen(
            [path.join(igtTestRoot, test), '--list-subtests'],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            env=os.environ.copy(),
            universal_newlines=True
            )
    out, err = proc.communicate()

    # a return code of 79 indicates there are no subtests
    if proc.returncode == 79:
         profile.test_list[path.join('igt', test)] = IGTTest(test)
         return

    if proc.returncode != 0:
         print "Error: Could not list subtests for " + test
         return

    subtests = out.split("\n")

    for subtest in subtests:
        if subtest == "":
            continue
        profile.test_list[path.join('igt', test, subtest)] = \
            IGTTest(test, ['--run-subtest', subtest])

for test in tests:
    addSubTestCases(test)

profile.dmesg = True

# the dmesg property of TestProfile returns a Dmesg object
profile.dmesg.regex = re.compile(r"(\[drm:|drm_|intel_|i915_)")
