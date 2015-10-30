# Copyright (c) 2015 Intel Corporation

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

from __future__ import absolute_import, division, print_function
import errno
import os
import subprocess

from framework import exceptions, core
from framework.options import OPTIONS
from .base import TestIsSkip

# pylint: disable=too-few-public-methods

__all__ = [
    'FastSkipMixin',
]


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
        return raw

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

        return {e.strip() for e in all_}


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

        super(FastSkipMixin, self).is_skip()
