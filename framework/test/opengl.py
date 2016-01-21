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

"""Mixins for OpenGL derived tests."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import errno
import os
import subprocess
import warnings

import six

from framework import exceptions, core
from framework.options import OPTIONS
from .base import TestIsSkip

# pylint: disable=too-few-public-methods

__all__ = [
    'FastSkipMixin',
]

# An environment variable that when set to true disables the FastSkipMixin by
# stubbing it out
_DISABLED = bool(os.environ.get('PIGLIT_NO_FAST_SKIP', False))


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

    This solves all of that, and is

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
                raw = subprocess.check_output(
                    ['wflinfo',
                     '--platform', OPTIONS.env['PIGLIT_PLATFORM']] + opts,
                    stderr=d)
            except subprocess.CalledProcessError:
                # When we hit this error it usually going to be because we have
                # an incompatible platform/profile combination
                raise StopWflinfo('Called')
            except OSError as e:
                # If we get a 'no wflinfo' warning then just return
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
                    # This means tat the particular api or profile is
                    # unsupported
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
        stopping when it finds a profile. It assumes taht most implementations
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


class FastSkipMixin(object):
    """Fast test skipping for OpenGL based suites.

    This provides an is_skip() method which will skip the test if an of it's
    requirements are not met.

    It also provides new attributes:
    gl_reqruied -- This is a set of extensions that are required for running
                   the extension.
    gl_version -- A float that is the required version number for an OpenGL
                  test.
    gles_version -- A float that is the required version number for an OpenGL
                    ES test
    glsl_version -- A float that is the required version number of OpenGL
                    Shader Language for a test
    glsl_ES_version -- A float that is the required version number of OpenGL ES
                       Shader Language for a test

    This requires wflinfo to be installed and accessible to provide it's
    functionality, however, it will no-op if wflinfo is not accessible.

    The design of this function is conservative. The design goal is that it
    it is better to run a few tests that could have been skipped, than to skip
    all the tests that could have, but also a few that should have run.

    """
    # XXX: This still gets called once for each thread. (4 times with 4
    # threads), this is a synchronization issue and I don't know how to stop it
    # other than querying each value before starting the thread pool.
    __info = WflInfo()

    def __init__(self, *args, **kwargs):
        super(FastSkipMixin, self).__init__(*args, **kwargs)
        self.gl_required = set()
        self.gl_version = None
        self.gles_version = None
        self.glsl_version = None
        self.glsl_es_version = None

    def is_skip(self):
        """Skip this test if any of it's feature requirements are unmet.

        If no extensions were calculated (if wflinfo isn't installed) then run
        all tests.

        """
        if self.__info.gl_extensions:
            for extension in self.gl_required:
                if extension not in self.__info.gl_extensions:
                    raise TestIsSkip(
                        'Test requires extension {} '
                        'which is not available'.format(extension))

        if (self.__info.gl_version is not None
                and self.gl_version is not None
                and self.gl_version > self.__info.gl_version):
            raise TestIsSkip(
                'Test requires OpenGL version {}, '
                'but only {} is available'.format(
                    self.gl_version, self.__info.gl_version))

        if (self.__info.gles_version is not None
                and self.gles_version is not None
                and self.gles_version > self.__info.gles_version):
            raise TestIsSkip(
                'Test requires OpenGL ES version {}, '
                'but only {} is available'.format(
                    self.gles_version, self.__info.gles_version))

        if (self.__info.glsl_version is not None
                and self.glsl_version is not None
                and self.glsl_version > self.__info.glsl_version):
            raise TestIsSkip(
                'Test requires OpenGL Shader Language version {}, '
                'but only {} is available'.format(
                    self.glsl_version, self.__info.glsl_version))

        if (self.__info.glsl_es_version is not None
                and self.glsl_es_version is not None
                and self.glsl_es_version > self.__info.glsl_es_version):
            raise TestIsSkip(
                'Test requires OpenGL ES Shader Language version {}, '
                'but only {} is available'.format(
                    self.glsl_es_version, self.__info.glsl_es_version))

        super(FastSkipMixin, self).is_skip()


class FastSkipMixinDisabled(object):
    def __init__(self, *args, **kwargs):
        # Tests that implement the FastSkipMixin expect to have these values
        # set, so just fill them in with the default values.
        self.gl_required = set()
        self.gl_version = None
        self.gles_version = None
        self.glsl_version = None
        self.glsl_es_version = None

        super(FastSkipMixinDisabled, self).__init__(*args, **kwargs)


# Shadow the real FastSkipMixin with the Disabled version if
# PIGLIT_NO_FAST_SKIP is truthy
if _DISABLED:
    warnings.warn('Fast Skipping Disabled')
    FastSkipMixin = FastSkipMixinDisabled
