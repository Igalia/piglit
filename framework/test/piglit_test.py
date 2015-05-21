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

from __future__ import print_function, absolute_import
import os
import sys
import glob
try:
    import simplejson as json
except ImportError:
    import json

from .base import Test, WindowResizeMixin, TestIsSkip
import framework.core as core


__all__ = [
    'PiglitCLTest',
    'PiglitGLTest',
    'CL_CONCURRENT',
    'TEST_BIN_DIR',
]

if 'PIGLIT_BUILD_DIR' in os.environ:
    TEST_BIN_DIR = os.path.join(os.environ['PIGLIT_BUILD_DIR'], 'bin')
else:
    TEST_BIN_DIR = os.path.normpath(os.path.join(os.path.dirname(__file__),
                                                 '../../bin'))

CL_CONCURRENT = (not sys.platform.startswith('linux') or
                 glob.glob('/dev/dri/render*'))


class PiglitBaseTest(Test):
    """
    PiglitTest: Run a "native" piglit test executable

    Expect one line prefixed PIGLIT: in the output, which contains a result
    dictionary. The plain output is appended to this dictionary
    """
    def __init__(self, command, run_concurrent=True, **kwargs):
        super(PiglitBaseTest, self).__init__(command, run_concurrent, **kwargs)

        # Prepend TEST_BIN_DIR to the path.
        self._command[0] = os.path.join(TEST_BIN_DIR, self._command[0])

    @Test.command.getter
    def command(self):
        command = super(PiglitBaseTest, self).command
        if self.OPTS.valgrind:
            return ['valgrind', '--quiet', '--error-exitcode=1',
                    '--tool=memcheck'] + command
        return command

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

    This class also provides two additional keyword arguments, require_platform
    and exclude_platforms. require_platforms may be set to a list of platforms
    which the test requires to run. This should be resereved for platform
    specific tests, such as GLX specific tests, or EGL specific tests. Multiple
    platforms are allowed because EGL can be fulfilled by multiple platforms.
    exclude_platforms is a list of platforms a test should not be run on, this
    is useful for tests that are valid on more than one platform, but not on
    all of them. This will probably be mainly used to exclude gbm. These
    options are mutually exclusive.

    """
    def __init__(self, command, require_platforms=None, exclude_platforms=None,
                 **kwargs):
        # TODO: There is a design flaw in python2, keyword args can be
        # fulfilled as positional arguments. This sounds really great, until
        # you realize that because of it you cannot use the splat operator with
        # args and create new keyword arguments.
        # What we really want is __init__(self, *args, new_arg=None, **kwargs),
        # but this doesn't work in python2. In python3 thanks to PEP3102, you
        # can in fact do just that
        # The work around is to explicitely pass the arguments down.
        super(PiglitGLTest, self).__init__(command, **kwargs)

        assert not (require_platforms and exclude_platforms)

        if not require_platforms or set(require_platforms).issubset(
                set(core.PLATFORMS)):
            self.__require_platforms = require_platforms or []
        else:
            raise Exception("Error: require_platform is not valid")

        if (not exclude_platforms or
                set(exclude_platforms).issubset(set(core.PLATFORMS))):
            self.__exclude_platforms = exclude_platforms or []
        else:
            raise Exception("Error: exclude_platforms is not valid")

    def is_skip(self):
        """ Native Piglit-test specific skip checking

        If the platform for the run doesn't support glx (either directly as
        glx or through the hybrid glx/x11_egl setup that is default), then skip
        any glx specific tests.

        """
        platform = self.OPTS.env['PIGLIT_PLATFORM']
        if self.__require_platforms and platform not in self.__require_platforms:
            raise TestIsSkip(
                'Test requires one of the following platforms "{}" '
                'but the platform is "{}"'.format(
                    self.__require_platforms, platform))
        elif self.__exclude_platforms and platform in self.__exclude_platforms:
            raise TestIsSkip(
                'Test cannot be run on any of the following platforms "{}" '
                'and the platform is "{}"'.format(
                    self.__exclude_platforms, platform))
        super(PiglitGLTest, self).is_skip()

    @PiglitBaseTest.command.getter
    def command(self):
        """ Automatically add -auto and -fbo as appropriate """
        if not self.run_concurrent:
            return super(PiglitGLTest, self).command + ['-auto']
        else:
            return super(PiglitGLTest, self).command + ['-auto', '-fbo']


class PiglitCLTest(PiglitBaseTest):  # pylint: disable=too-few-public-methods
    """ OpenCL specific Test class.

    Set concurrency based on CL requirements.

    """
    def __init__(self, command, run_concurrent=CL_CONCURRENT, **kwargs):
        super(PiglitCLTest, self).__init__(command, run_concurrent, **kwargs)
