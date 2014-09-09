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

from __future__ import print_function
import re
import subprocess
from os import path
from framework.gtest import GTest
from framework.core import PIGLIT_CONFIG


class OpenCVTest(GTest):
    def __init__(self, test_prog, testname):
        options = [test_prog, '--gtest_filter=' + testname, '--gtest_color=no']
        if PIGLIT_CONFIG.has_option('opencv', 'workdir'):
            options.append('-w {}'.format(PIGLIT_CONFIG.get('opencv', 'workdir')))
        GTest.__init__(self, options)


def add_opencv_tests(profile):
    if not PIGLIT_CONFIG.has_option('opencv', 'opencv_test_ocl_bindir'):
        return

    opencv_test_ocl = path.join(PIGLIT_CONFIG.get('opencv',
        'opencv_test_ocl_bindir'), 'opencv_test_ocl')
    individual = PIGLIT_CONFIG.has_option('opencv', 'individual')
    if not path.isfile(opencv_test_ocl):
        print('Warning: {} does not exist.\nSkipping OpenCV '
              'tests...'.format(opencv_test_ocl))
        return
    tests = subprocess.check_output([opencv_test_ocl, '--gtest_list_tests'])
    test_list = tests.splitlines()
    group_name = ''
    full_test_name = ''
    for line in test_list:
        #Test groups names start at the beginning of the line and end with '.'
        m = re.match('([^.]+\.)$', line)
        if m:
            group_name = m.group(1)
            group_desc = group_name[:-1]
            full_test_name = 'opencv/{}'.format(group_desc)
            if not individual:
                profile.tests[full_test_name] = OpenCVTest(opencv_test_ocl,
                    '{}*'.format(group_name))
            continue

        if not individual:
            continue

        # Test names are indent by 2 spaces
        m = re.match('  ([^ ]+)', line)
        if m:
            test_name = m.group(1)
            profile.tests['{}/{}'.format(full_test_name,test_name)] = \
                OpenCVTest(opencv_test_ocl, '{}{}'.format(group_name ,test_name))
