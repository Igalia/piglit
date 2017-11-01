# Copyright (c) 2015-2016 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import errno
import os
import subprocess
import sys

import six

from framework import exceptions, core
from framework.options import OPTIONS
from framework.test import piglit_test


class StopWflinfo(exceptions.PiglitException):
    """Exception called when wlfinfo getter should stop."""
    def __init__(self, reason):
        super(StopWflinfo, self).__init__()
        self.reason = reason


class WflInfo(object):
    """Class representing platform information as provided by wflinfo.

    The design of this is odd to say the least, it's basically a bag with some
    lazy property evaluators in it, used to avoid calculating the values
    provided by wflinfo more than once.

    The problems:
    - Needs to be shared with all subclasses
    - Needs to evaluate only once
    - cannot evaluate until user sets OPTIONS.env['PIGLIT_PLATFORM']

    This solves all of that.

    """
    __shared_state = {}
    def __new__(cls, *args, **kwargs):
        # Implement the borg pattern:
        # https://code.activestate.com/recipes/66531-singleton-we-dont-need-no-stinkin-singleton-the-bo/
        #
        # This is something like a singleton, but much easier to implement
        self = super(WflInfo, cls).__new__(cls, *args, **kwargs)
        self.__dict__ = cls.__shared_state
        return self

    @staticmethod
    def __call_wflinfo(opts):
        """Helper to call wflinfo and reduce code duplication.

        This catches and handles CalledProcessError and OSError.ernno == 2
        gracefully: it passes them to allow platforms without a particular
        gl/gles version or wflinfo (resepctively) to work.

        Arguments:
        opts -- arguments to pass to wflinfo other than verbose and platform

        """
        with open(os.devnull, 'w') as d:
            try:
                # Get the piglit platform string and, if needed, convert it
                # to something that wflinfo understands.
                platform = OPTIONS.env['PIGLIT_PLATFORM']
                if platform == "mixed_glx_egl":
                    platform = "glx"

                if sys.platform in ['windows', 'cygwin']:
                    bin = 'wflinfo.exe'
                else:
                    bin = 'wflinfo'

                cmd = [bin, '--platform', platform] + opts

                # setup execution environment where we extend the PATH env var
                # to include the piglit TEST_BIN_DIR
                new_env = os.environ
                new_env['PATH'] = ':'.join([piglit_test.TEST_BIN_DIR,
                                            os.environ['PATH']])

                raw = subprocess.check_output(cmd, env=new_env, stderr=d)

            except subprocess.CalledProcessError:
                # When we hit this error it usually going to be because we have
                # an incompatible platform/profile combination
                raise StopWflinfo('Called')
            except OSError as e:
                # If we get a 'no wflinfo' warning then just return
                print("wflinfo utility not found.", file=sys.stderr)
                if e.errno == errno.ENOENT:
                    raise StopWflinfo('OSError')
                raise
        return raw.decode('utf-8')

    @staticmethod
    def __getline(lines, name):
        """Find a line in a list return it."""
        for line in lines:
            if line.startswith(name):
                return line
        raise Exception('Unreachable')

    @core.lazy_property
    def gl_extensions(self):
        """Call wflinfo to get opengl extensions.

        This provides a very conservative set of extensions, it provides every
        extension from gles1, 2 and 3 and from GL both core and compat profile
        as a single set. This may let a few tests execute that will still skip
        manually, but it helps to ensure that this method never skips when it
        shouldn't.

        """
        _trim = len('OpenGL extensions: ')
        all_ = set()

        def helper(const, vars_):
            """Helper function to reduce code duplication."""
            # This is a pretty fragile function but it really does help with
            # duplication
            for var in vars_:
                try:
                    ret = self.__call_wflinfo(const + [var])
                except StopWflinfo as e:
                    # This means the particular api or profile is unsupported
                    if e.reason == 'Called':
                        continue
                    else:
                        raise
                all_.update(set(self.__getline(
                    ret.split('\n'), 'OpenGL extensions')[_trim:].split()))

        try:
            helper(['--verbose', '--api'], ['gles1', 'gles2', 'gles3'])
            helper(['--verbose', '--api', 'gl', '--profile'],
                   ['core', 'compat', 'none'])
        except StopWflinfo as e:
            # Handle wflinfo not being installed by returning an empty set. This
            # will essentially make FastSkipMixin a no-op.
            if e.reason == 'OSError':
                return set()
            raise

        # Don't return a set with only WFLINFO_GL_ERROR.
        ret = {e.strip() for e in all_}
        if ret == {'WFLINFO_GL_ERROR'}:
            return set()
        return ret

    @core.lazy_property
    def gl_version(self):
        """Calculate the maximum opengl version.

        This will try (in order): core, compat, and finally no profile,
        stopping when it finds a profile. It assumes that most implementations
        will have core and compat as equals, or core as superior to compat in
        terms of support.

        """
        ret = None
        for profile in ['core', 'compat', 'none']:
            try:
                raw = self.__call_wflinfo(['--api', 'gl', '--profile', profile])
            except StopWflinfo as e:
                if e.reason == 'Called':
                    continue
                elif e.reason == 'OSError':
                    break
                raise
            else:
                try:
                    # Grab the GL version string, trim any release_number values
                    ret = float(self.__getline(
                        raw.split('\n'),
                        'OpenGL version string').split()[3][:3])
                except (IndexError, ValueError):
                    # This is caused by wlfinfo returning an error
                    pass
                break
        return ret

    @core.lazy_property
    def gles_version(self):
        """Calculate the maximum opengl es version.

        The design of this function isn't 100% correct. GLES1 and GLES2+ behave
        differently, since 2+ can be silently promoted, but 1 cannot. This
        means that a driver can implement 2, 3, 3.1, etc, but never have 1
        support.

        I don't think this is a big deal for a couple of reasons. First, piglit
        has a very small set of GLES1 tests, so they shouldn't have big impact
        on runtime, and second, the design of the FastSkipMixin is
        conservative: it would rather run a few tests that should be skipped
        than skip a few tests that should be run.

        """
        ret = None
        for api in ['gles3', 'gles2', 'gles1']:
            try:
                raw = self.__call_wflinfo(['--api', api])
            except StopWflinfo as e:
                if e.reason == 'Called':
                    continue
                elif e.reason == 'OSError':
                    break
                raise
            else:
                try:
                    # Yes, search for "OpenGL version string" in GLES
                    # GLES doesn't support patch versions.
                    ret = float(self.__getline(
                        raw.split('\n'),
                        'OpenGL version string').split()[5])
                except (IndexError, ValueError):
                    # This is caused by wlfinfo returning an error
                    pass
                break
        return ret

    @core.lazy_property
    def glsl_version(self):
        """Calculate the maximum OpenGL Shader Language version."""
        ret = None
        for profile in ['core', 'compat', 'none']:
            try:
                raw = self.__call_wflinfo(
                    ['--verbose', '--api', 'gl', '--profile', profile])
            except StopWflinfo as e:
                if e.reason == 'Called':
                    continue
                elif e.reason == 'OSError':
                    break
                raise
            else:
                try:
                    # GLSL versions are M.mm formatted
                    ret = float(self.__getline(
                        raw.split('\n'),
                        'OpenGL shading language').split()[-1][:4])
                except (IndexError, ValueError):
                    # This is caused by wflinfo returning an error
                    pass
                break
        return ret

    @core.lazy_property
    def glsl_es_version(self):
        """Calculate the maximum OpenGL ES Shader Language version."""
        ret = None
        for api in ['gles3', 'gles2']:
            try:
                raw = self.__call_wflinfo(['--verbose', '--api', api])
            except StopWflinfo as e:
                if e.reason == 'Called':
                    continue
                elif e.reason == 'OSError':
                    break
                raise
            else:
                try:
                    # GLSL ES version numbering is insane.
                    # For version >= 3 the numbers are 3.00, 3.10, etc.
                    # For version 2, they are 1.0.xx
                    ret = float(self.__getline(
                        raw.split('\n'),
                        'OpenGL shading language').split()[-1][:3])
                except (IndexError, ValueError):
                    # Handle wflinfo internal errors
                    pass
                break
        return ret

