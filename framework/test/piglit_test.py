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

""" Module provides a base class for Tests """

import os
try:
    import simplejson as json
except ImportError:
    import json

from .base import Test, WindowResizeMixin


__all__ = [
    'PiglitGLTest',
    'PiglitCLTest',
    'TEST_BIN_DIR'
]

if 'PIGLIT_BUILD_DIR' in os.environ:
    TEST_BIN_DIR = os.path.join(os.environ['PIGLIT_BUILD_DIR'], 'bin')
else:
    TEST_BIN_DIR = os.path.normpath(os.path.join(os.path.dirname(__file__),
                                                 '../../bin'))


class PiglitBaseTest(Test):
    """
    PiglitTest: Run a "native" piglit test executable

    Expect one line prefixed PIGLIT: in the output, which contains a result
    dictionary. The plain output is appended to this dictionary
    """
    def __init__(self, *args, **kwargs):
        super(PiglitBaseTest, self).__init__(*args, **kwargs)

        # Prepend TEST_BIN_DIR to the path.
        self._command[0] = os.path.join(TEST_BIN_DIR, self._command[0])

    def interpret_result(self):
        outlines = self.result['out'].split('\n')
        outpiglit = (s[7:] for s in outlines if s.startswith('PIGLIT:'))

        for piglit in outpiglit:
            self.result.recursive_update(json.loads(piglit))
        self.result['out'] = '\n'.join(
            s for s in outlines if not s.startswith('PIGLIT:'))


class PiglitGLTest(WindowResizeMixin, PiglitBaseTest):
    """ OpenGL specific Piglit test class

    This Subclass provides provides an is_skip() implementation that skips glx
    tests on non-glx platforms

    """
    def is_skip(self):
        """ Native Piglit-test specific skip checking

        If the platform for the run doesn't suppoprt glx (either directly as
        glx or through the hybrid glx/x11_egl setup that is default), then skip
        any glx specific tests.

        """
        if self.OPTS.env['PIGLIT_PLATFORM'] not in ['glx', 'mixed_glx_egl']:
            split_command = os.path.split(self._command[0])[1]
            if split_command.startswith('glx-'):
                return True
        return False


class PiglitCLTest(PiglitBaseTest):
    """ OpenCL specific Test class """
    pass
