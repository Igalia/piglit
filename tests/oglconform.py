#!/usr/bin/env python
# coding=utf-8
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

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import re
import subprocess
import tempfile

from framework import grouptools, exceptions, core
from framework.profile import TestProfile
from framework.test.base import Test

__all__ = ['profile']

BIN = core.PIGLIT_CONFIG.required_get('oglconform', 'path')

if not os.path.exists(BIN):
    raise exceptions.PiglitFatalError(
        'Cannot find binary {}'.format(BIN))


class OGLCTest(Test):
    """OGLCTest: Execute a sub-test of the Intel oglconform test suite.

    To use this, create an 'oglconform' symlink in piglit/bin.  Piglit
    will obtain a list of tests from oglconform and add them all.

    """
    skip_re = re.compile(
        r'no test in schedule is compat|'
        r'GLSL [13].[345]0 is not supported|'
        r'wont be scheduled due to lack of compatible fbconfig')

    def __init__(self, category, subtest):
        super(OGLCTest, self).__init__([category, subtest])

    @Test.command.getter
    def command(self):
        return [BIN, '-minFmt', '-v', '4', '-test'] + \
            super(OGLCTest, self).command

    def interpret_result(self):
        # Most of what we want to search for is in the last three lines of the
        # the output
        split = self.result.out.rsplit('\n', 4)[1:]
        if 'Total Passed : 1' in split:
            self.result.result = 'pass'
        elif 'Total Failed : 1' in split:
            # This is a fast path to avoid the regular expression.
            self.result.result = 'fail'
        elif ('Total Not run: 1' in split or
              self.skip_re.search(self.result.out) is not None):
            # Lazy evaluation means that the re (which is slow) is only tried if
            # the more obvious case is not true
            self.result.result = 'skip'
        else:
            self.result.result = 'fail'

        super(OGLCTest, self).interpret_result()


def _make_profile():
    """Create and populate a TestProfile instance."""
    profile_ = TestProfile()

    with tempfile.NamedTemporaryFile() as f:
        with open(os.devnull, "w") as d:
            subprocess.call([BIN, '-generateTestList', f.name],
                            stdout=d, stderr=d)

        f.seek(0)

        for l in f.readlines():
            try:
                category, test = l.split()
            except ValueError:
                continue

            group = grouptools.join('oglconform', category, test)
            profile_.test_list[group] = OGLCTest(category, test)

    return profile_


profile = _make_profile()  # pylint: disable=invalid-name
