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

""" Glean support """

import os

from .exectest import Test, TEST_BIN_DIR


# GleanTest: Execute a sub-test of Glean
class GleanTest(Test):
    """ Execute a glean subtest

    This class descendes from exectest.Test, and provides methods for running
    glean tests.

    """
    GLOBAL_PARAMS = []
    _EXECUTABLE = os.path.join(TEST_BIN_DIR, "glean")

    def __init__(self, name, **kwargs):
        super(GleanTest, self).__init__(
            [self._EXECUTABLE, "-o", "-v", "-v", "-v", "-t", "+" + name],
            **kwargs)

    @Test.command.getter
    def command(self):
        return super(GleanTest, self).command + self.GLOBAL_PARAMS

    def interpret_result(self):
        if self.result['returncode'] != 0 or 'FAIL' in self.result['out']:
            self.result['result'] = 'fail'
        else:
            self.result['result'] = 'pass'
