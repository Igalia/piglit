# coding=utf-8
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
import threading

import six

from framework import exceptions
from framework.core import lazy_property
from framework.options import OPTIONS
# from framework.test import piglit_test


class StopWflinfo(exceptions.PiglitException):
    """Exception called when wlfinfo getter should stop."""
    def __init__(self, reason):
        super(StopWflinfo, self).__init__()
        self.reason = reason


class ProfileInfo(object):
    """Information about a single profile (core, compat, es1, es2, etc)."""

    def __init__(self, shader_version, language_version, extensions):
        self.shader_version = shader_version
        self.api_version = language_version
        self.extensions = extensions


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
    __core_init = False
    __core_lock = threading.Lock()
    __compat_init = False
    __compat_lock = threading.Lock()
    __es1_init = False
    __es1_lock = threading.Lock()
    __es2_init = False
    __es2_lock = threading.Lock()

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
                new_env = os.environ.copy()
                # new_env['PATH'] = ':'.join([piglit_test.TEST_BIN_DIR,
                                            # os.environ['PATH']])

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

    def __get_shader_version(self, profile):
        """Calculate the maximum OpenGL Shader Language version."""
        ret = 0.0
        if profile in ['core', 'compat', 'none']:
            try:
                raw = self.__call_wflinfo(
                    ['--verbose', '--api', 'gl', '--profile', profile])
            except StopWflinfo as e:
                if e.reason not in ['Called', 'OSError']:
                    raise
            else:
                try:
                    # GLSL versions are M.mm formatted
                    line = self.__getline(raw.split('\n'), 'OpenGL shading language')
                    ret = float(line.split(":")[1][:5])
                except (IndexError, ValueError):
                    # This is caused by wflinfo returning an error
                    pass
        elif profile in ['gles2', 'gles3']:
            try:
                raw = self.__call_wflinfo(['--verbose', '--api', profile])
            except StopWflinfo as e:
                if e.reason not in ['Called', 'OSError']:
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
        return ret

    def __get_language_version(self, profile):
        ret = 0.0
        if profile in ['core', 'compat', 'none']:
            try:
                raw = self.__call_wflinfo(['--api', 'gl', '--profile', profile])
            except StopWflinfo as e:
                if e.reason not in ['Called', 'OSError']:
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
        else:
            try:
                raw = self.__call_wflinfo(['--api', profile])
            except StopWflinfo as e:
                if e.reason not in ['Called', 'OSError']:
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
        return ret

    def __get_extensions(self, profile):
        """Call wflinfo to get opengl extensions.

        This provides a very conservative set of extensions, it provides every
        extension from gles1, 2 and 3 and from GL both core and compat profile
        as a single set. This may let a few tests execute that will still skip
        manually, but it helps to ensure that this method never skips when it
        shouldn't.

        """
        _trim = len('OpenGL extensions: ')
        all_ = set()

        def helper(args):
            """Helper function to reduce code duplication."""
            # This is a pretty fragile function but it really does help with
            # duplication
            ret = self.__call_wflinfo(args)
            all_.update(set(self.__getline(
                ret.split('\n'), 'OpenGL extensions')[_trim:].split()))

        try:
            if profile in ['core', 'compat', 'none']:
                helper(['--verbose', '--api', 'gl', '--profile', profile])
            else:
                helper(['--verbose', '--api', profile])
        except StopWflinfo as e:
            # Handle wflinfo not being installed by returning an empty set. This
            # will essentially make FastSkipMixin a no-op.
            if e.reason in ['OSError', 'Called']:
                return set()
            raise

        # Don't return a set with only WFLINFO_GL_ERROR.
        ret = {e.strip() for e in all_}
        if ret == {'WFLINFO_GL_ERROR'}:
            return set()
        return ret

    def __build_info(self, profile):
        return ProfileInfo(
            self.__get_shader_version(profile),
            self.__get_language_version(profile),
            self.__get_extensions(profile)
        )

    @lazy_property
    def core(self):
        with self.__core_lock:
            if not self.__core_init:
                self.__core_init = True
                return self.__build_info('core')
        return self.core

    @lazy_property
    def compat(self):
        with self.__compat_lock:
            if not self.__compat_init:
                self.__compat_init = True
                comp = self.__build_info('compat')
                if comp.api_version == 0.0:
                    # In this case there are not compat profiles, try agian
                    # with a "legacy" profile, which could be promoted to
                    # compat
                    return self.__build_info('none')
                return comp
        return self.compat

    @lazy_property
    def es1(self):
        with self.__es1_lock:
            if not self.__es1_init:
                self.__es1_init = True
                return self.__build_info('gles1')
        return self.es1

    @lazy_property
    def es2(self):
        with self.__es2_lock:
            if not self.__es2_init:
                self.__es2_init = True
                return self.__build_info('gles2')
        return self.es2

    @lazy_property
    def es3(self):
        return self.es2
