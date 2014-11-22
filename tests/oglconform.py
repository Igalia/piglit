#!/usr/bin/env python
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

import framework.grouptools as grouptools
import framework.core
from framework.profile import TestProfile, Test

__all__ = ['profile']

bin_oglconform = framework.core.PIGLIT_CONFIG.get('oglconform', 'path')

if not os.path.exists(bin_oglconform):
    sys.exit(0)

profile = TestProfile()

#############################################################################
##### OGLCTest: Execute a sub-test of the Intel oglconform test suite.
#####
##### To use this, create an 'oglconform' symlink in piglit/bin.  Piglit
##### will obtain a list of tests from oglconform and add them all.
#############################################################################
class OGLCTest(Test):
    skip_re = re.compile(r'Total Not run: 1|no test in schedule is compat|GLSL [13].[345]0 is not supported|wont be scheduled due to lack of compatible fbconfig')

    def __init__(self, category, subtest):
        super(OGLCTest, self).__init__([bin_oglconform, '-minFmt', '-v', '4',
                                        '-test', category, subtest])

    def interpret_result(self):
        if self.skip_re.search(self.result['out']) is not None:
            self.result['result'] = 'skip'
        elif re.search('Total Passed : 1', self.result['out']) is not None:
            self.result['result'] = 'pass'
        else:
            self.result['result'] = 'fail'

# Create a new top-level 'oglconform' category

testlist_file = '/tmp/oglc.tests'

with open(os.devnull, "w") as devnull:
    subprocess.call([bin_oglconform, '-generateTestList', testlist_file], stdout=devnull.fileno(), stderr=devnull.fileno())

with open(testlist_file) as f:
    testlist = f.read().splitlines()
    for l in testlist:
        try:
            category, test = l.split()
            profile.test_list[grouptools.join('oglconform', category, test)] = OGLCTest(category, test)
        except:
            continue
