# Copyright 2014 Advanced Micro Devices, Inc.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Authors: Tom Stellard <thomas.stellard@amd.com>
#

from __future__ import print_function, print_function
import re
import subprocess
from os.path import join
from sys import stderr

import framework.grouptools as grouptools
from framework.core import PIGLIT_CONFIG
from .base import Test

__all__ = [
    'OCLConform',
    'add_oclconform_tests',
]


def get_test_section_name(test):
    return 'oclconform-{}'.format(test)

class OCLConform(Test):

    def interpret_result(self):
        if self.result['returncode'] != 0 or 'FAIL' in self.result['out']:
            self.result['result'] = 'fail'
        else:
            self.result['result'] = 'pass'

def add_sub_test(profile, test_name, subtest_name, subtest):
    profile.tests[grouptools.join('oclconform', test_name, subtest_name)] = subtest

def add_test(profile, test_name, test):
    profile.tests[grouptools.join('oclconform', test_name)] = test

def add_oclconform_tests(profile):
    section_name = 'oclconform'
    if not PIGLIT_CONFIG.has_section(section_name):
        return

    bindir = PIGLIT_CONFIG.get(section_name, 'bindir')
    options = PIGLIT_CONFIG.options(section_name)

    tests = (o for o in options if PIGLIT_CONFIG.get(section_name, o) is None)

    for test in tests:
        test_section_name = get_test_section_name(test)
        if not PIGLIT_CONFIG.has_section(test_section_name):
            print("Warning: no section defined for {}".format(test), file=stderr)
            continue

        test_name = PIGLIT_CONFIG.get(test_section_name, 'test_name')
        should_run_concurrent = PIGLIT_CONFIG.has_option(test_section_name, 'concurrent')
        if PIGLIT_CONFIG.has_option(test_section_name, 'list_subtests'):
            list_tests = PIGLIT_CONFIG.get(test_section_name, 'list_subtests')
            subtest_regex = PIGLIT_CONFIG.get(test_section_name, 'subtest_regex')
            subtest_regex.encode('string_escape')
            run_subtests = PIGLIT_CONFIG.get(test_section_name, 'run_subtest')
            list_tests =list_tests.split()

            subtests = subprocess.check_output(args=list_tests, cwd=bindir).split('\n')
            for subtest in subtests:
                m = re.match(subtest_regex, subtest)
                if not m:
                    continue
                subtest = m.group(1)
                subtest_command = join(bindir, run_subtests.replace('<subtest>', subtest))
                add_sub_test(profile, test_name, subtest,
		             OCLConform(command=subtest_command.split(),
			                run_concurrent=should_run_concurrent))
        else:
            run_test = PIGLIT_CONFIG.get(test_section_name, 'run_test')
            add_test(profile, test_name, OCLConform(command=run_test.split(),
	                                            run_concurrent=should_run_concurrent))

