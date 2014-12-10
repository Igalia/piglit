# Copyright 2013, 2014 Advanced Micro Devices, Inc.
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

from __future__ import print_function, absolute_import 
import re

from .base import Test

__all__ = [
    'GTest',
]


class GTest(Test):
    def interpret_result(self):
        # Since gtests can have several subtets, if any of the subtests fail
        # then we need to report fail.
        out = self.result['out']
        if len(re.findall('FAILED', out, re.MULTILINE)) > 0:
            self.result['result'] = 'fail'
        elif len(re.findall('PASSED', out, re.MULTILINE)) > 0:
            self.result['result'] = 'pass'
        else:
            #If we get here, then the test probably exited early.
            self.result['result'] = 'fail'
        return out
