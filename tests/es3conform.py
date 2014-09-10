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

from os import path
from glob import glob
from framework.profile import TestProfile
from framework.test import Test, TEST_BIN_DIR

__all__ = ['profile']

#############################################################################
##### GTFTest: Execute a sub-test of the Khronos ES 3.0 Conformance suite.
#####
##### To use this, create a 'GTF3' symlink in piglit/bin which points to the
##### Khronos 'GTF' executable.  Piglit will automatically add all .test
##### files into the 'gtf' category.
#############################################################################

if not path.exists(path.join(TEST_BIN_DIR, 'GTF3')):
    sys.exit(0)

profile = TestProfile()

# Chase the piglit/bin/GTF symlink to find where the tests really live.
gtfroot = path.dirname(path.realpath(path.join(TEST_BIN_DIR, 'GTF3')))

class GTFTest(Test):
    pass_re = re.compile(r'(Conformance|Regression) PASSED all (?P<passed>\d+) tests')

    def __init__(self, testpath):
        super(GTFTest, self).__init__([path.join(TEST_BIN_DIR, 'GTF3'),
                                       '-minfmt', '-width=113', '-height=47',
                                       '-run=' + testpath])

    def interpret_result(self):
        mo = self.pass_re.search(self.result['out'])
        if mo is not None and int(mo.group('passed')) > 0:
            self.result['result'] = 'pass'
        else:
            self.result['result'] = 'fail'

def populateTests(runfile):
    "Read a .run file, adding any .test files to the profile"
    with open(runfile, 'r') as f:
        for line in f.readlines():
            # Ignore comments and whitespace
            line = line.strip()
            if line.startswith('#') or line == '':
                continue

            newpath = path.join(path.dirname(runfile), line)
            if line.endswith('.run'):
                populateTests(newpath)
            else:
                # Add the .test file
                group = path.join('es3conform', path.relpath(newpath, gtfroot))
                profile.test_list[group] = GTFTest(newpath)


# Populate the group with all the .test files
populateTests(path.join(gtfroot, 'mustpass_es30.run'))
