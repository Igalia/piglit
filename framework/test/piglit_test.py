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

""" Module provides a base class for Tests """

import glob
import os
import sys
try:
    import simplejson as json
except ImportError:
    import json

from framework import core, options
from framework import status
from .base import Test, WindowResizeMixin, ValgrindMixin, TestIsSkip


__all__ = [
    'PiglitCLTest',
    'PiglitGLTest',
    'PiglitBaseTest',
    'VkRunnerTest',
    'CL_CONCURRENT',
    'ROOT_DIR',
    'TEST_BIN_DIR',
]

if 'PIGLIT_BUILD_DIR' in os.environ:
    ROOT_DIR = os.environ['PIGLIT_BUILD_DIR']
else:
    ROOT_DIR = os.path.normpath(os.path.join(os.path.dirname(__file__), '../..'))

TEST_BIN_DIR = os.path.normpath(os.path.join(ROOT_DIR, 'bin'))

CL_CONCURRENT = bool(not sys.platform.startswith('linux') or
                     glob.glob('/dev/dri/render*'))


class PiglitBaseTest(ValgrindMixin, Test):
    """
    PiglitTest: Run a "native" piglit test executable

    Expect one line prefixed PIGLIT: in the output, which contains a result
    dictionary. The plain output is appended to this dictionary
    """
    def __init__(self, command, run_concurrent=True, **kwargs):
        super(PiglitBaseTest, self).__init__(command, run_concurrent, **kwargs)

    @Test.command.getter
    def command(self):
        command = super(PiglitBaseTest, self).command

        def fixup_bin_path(c):
            # Prepend TEST_BIN_DIR to the path.
            if c == self._command[0]:
                return os.path.join(TEST_BIN_DIR, c)
            else:
                return c

        return [fixup_bin_path(c) for c in command]

    def interpret_result(self):
        out = []

        for each in self.result.out.split('\n'):
            if each.startswith('PIGLIT:'):
                deserial = json.loads(each[8:])
                if 'enumerate subtests' in deserial:
                    for n in deserial['enumerate subtests']:
                        self.result.subtests[n] = status.NOTRUN
                else:
                    self.result.update(deserial)
            else:
                out.append(each)

        self.result.out = '\n'.join(out)

        # XXX: There are a number of tests that now enumerate their subtests,
        # but don't properly report skip for all of them when the skip due to a
        # missing feature. We should fix these tests to do the right thing, but
        # for the moment this work around will suffice to keep things running.
        if self.result.raw_result is status.SKIP and self.result.subtests:
            for k, v in self.result.subtests.items():
                if v is status.NOTRUN:
                    self.result.subtests[k] = status.SKIP

        super(PiglitBaseTest, self).interpret_result()


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
            self.require_platforms = require_platforms or []
        else:
            raise Exception("Error: require_platform is not valid")

        if (not exclude_platforms or
                set(exclude_platforms).issubset(set(core.PLATFORMS))):
            self.exclude_platforms = exclude_platforms or []
        else:
            raise Exception("Error: exclude_platforms is not valid")

    def is_skip(self):
        """ Native Piglit-test specific skip checking

        If the platform for the run doesn't support glx (either directly as
        glx or through the hybrid glx/x11_egl setup that is default), then skip
        any glx specific tests.

        """
        platform = options.OPTIONS.env['PIGLIT_PLATFORM']
        if self.require_platforms and platform not in self.require_platforms:
            raise TestIsSkip(
                'Test requires one of the following platforms "{}" '
                'but the platform is "{}"'.format(
                    self.require_platforms, platform))
        elif self.exclude_platforms and platform in self.exclude_platforms:
            raise TestIsSkip(
                'Test cannot be run on any of the following platforms "{}" '
                'and the platform is "{}"'.format(
                    self.exclude_platforms, platform))
        super(PiglitGLTest, self).is_skip()

    @PiglitBaseTest.command.getter
    def command(self):
        """ Automatically add -auto and -fbo as appropriate """
        if not self.run_concurrent:
            return super(PiglitGLTest, self).command + ['-auto']
        else:
            return super(PiglitGLTest, self).command + ['-auto', '-fbo']

    @command.setter
    def command(self, new):
        self._command = [n for n in new if n not in ['-auto', '-fbo']]


class ASMParserTest(PiglitBaseTest):

    """Test class for ASM parser tests."""

    def __init__(self, type_, filename, env=None):
        super(ASMParserTest, self).__init__(['asmparsertest', type_], env=env)
        self.filename = filename

    @PiglitBaseTest.command.getter
    def command(self):
        command = super(ASMParserTest, self).command
        return command + [os.path.join(ROOT_DIR, self.filename)]


class BuiltInConstantsTest(PiglitBaseTest):

    """Test class for handling built in constants tests."""

    @PiglitBaseTest.command.getter
    def command(self):
        command = super(BuiltInConstantsTest, self).command
        command[1] = os.path.join(ROOT_DIR, 'tests', command[1])
        return command


class PiglitCLTest(PiglitBaseTest):  # pylint: disable=too-few-public-methods
    """ OpenCL specific Test class.

    Set concurrency based on CL requirements.

    """
    def __init__(self, command, run_concurrent=CL_CONCURRENT, **kwargs):
        if self.timeout is None:
            self.timeout = 60
        super(PiglitCLTest, self).__init__(command, run_concurrent, **kwargs)


class CLProgramTester(PiglitCLTest):

    """Class for cl-program-tester tests."""

    def __init__(self, filename, **kwargs):
        super(CLProgramTester, self).__init__(['cl-program-tester'], **kwargs)
        self.filename = filename

    @PiglitCLTest.command.getter
    def command(self):
        command = super(CLProgramTester, self).command
        return command + [os.path.join(ROOT_DIR, self.filename)]


class VkRunnerTest(PiglitBaseTest):
    """ Make a PiglitTest instance for a VkRunner shader test file """

    def __init__(self, filename, env=None):
        vkrunner_bin = core.get_option('PIGLIT_VKRUNNER_BINARY',
                                       ('vkrunner', 'bin'),
                                       default='vkrunner')

        super(VkRunnerTest, self).__init__(
            [vkrunner_bin],
            run_concurrent=True,
            env=env)

        self.filename = filename

    @PiglitBaseTest.command.getter
    def command(self):
        # self._command is used because we don't want PiglitBaseTest
        # to prepend TEST_BIN_DIR so that it will look for vkrunner in
        # the search path.
        return self._command + [os.path.join(ROOT_DIR, self.filename)]
