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
import os
import warnings

import six

from framework import wflinfo
from .base import TestIsSkip

# pylint: disable=too-few-public-methods

__all__ = [
    'FastSkip',
    'FastSkipMixin',
]


class FastSkip(object):
    """A class for testing OpenGL requirements.

    This class provides a mechanism for testing OpenGL requirements, and
    skipping tests that have unmet requirements

    This requires wflinfo to be installed and accessible to provide its
    functionality, however, it will no-op if wflinfo is not accessible.

    The design of this function is conservative. The design goal is that it
    it is better to run a few tests that could have been skipped, than to skip
    all the tests that could have, but also a few that should have run.

    Keyword Arguments:
    gl_required     -- This is a set of extensions that are required for
                       running the test.
    gl_version      -- A float that is the required version number for an
                       OpenGL test.
    gles_version    -- A float that is the required version number for an
                       OpenGL ES test
    glsl_version    -- A float that is the required version number of OpenGL
                       Shader Language for a test
    glsl_es_version -- A float that is the required version number of OpenGL ES
                       Shader Language for a test
    """
    __slots__ = ['gl_required', 'gl_version', 'gles_version', 'glsl_version',
                 'glsl_es_version']

    info = wflinfo.WflInfo()

    def __init__(self, gl_required=None, gl_version=None, gles_version=None,
                 glsl_version=None, glsl_es_version=None):
        self.gl_required = gl_required or set()
        self.gl_version = gl_version
        self.gles_version = gles_version
        self.glsl_version = glsl_version
        self.glsl_es_version = glsl_es_version

    def test(self):
        """Skip this test if any of its feature requirements are unmet.

        If no extensions were calculated (if wflinfo isn't installed) then run
        all tests.

        Raises:
        TestIsSkip   -- if any of the conditions passed to self are false
        """
        if self.info.gl_extensions:
            for extension in self.gl_required:
                if extension not in self.info.gl_extensions:
                    raise TestIsSkip(
                        'Test requires extension {} '
                        'which is not available'.format(extension))

        # TODO: Be able to handle any operator
        if (self.info.gl_version is not None
                and self.gl_version is not None
                and self.gl_version > self.info.gl_version):
            raise TestIsSkip(
                'Test requires OpenGL version {}, '
                'but only {} is available'.format(
                    self.gl_version, self.info.gl_version))

        # TODO: Be able to handle any operator
        if (self.info.gles_version is not None
                and self.gles_version is not None
                and self.gles_version > self.info.gles_version):
            raise TestIsSkip(
                'Test requires OpenGL ES version {}, '
                'but only {} is available'.format(
                    self.gles_version, self.info.gles_version))

        # TODO: Be able to handle any operator
        if (self.info.glsl_version is not None
                and self.glsl_version is not None
                and self.glsl_version > self.info.glsl_version):
            raise TestIsSkip(
                'Test requires OpenGL Shader Language version {}, '
                'but only {} is available'.format(
                    self.glsl_version, self.info.glsl_version))

        # TODO: Be able to handle any operator
        if (self.info.glsl_es_version is not None
                and self.glsl_es_version is not None
                and self.glsl_es_version > self.info.glsl_es_version):
            raise TestIsSkip(
                'Test requires OpenGL ES Shader Language version {}, '
                'but only {} is available'.format(
                    self.glsl_es_version, self.info.glsl_es_version))


class FastSkipMixin(object):
    """Fast test skipping for OpenGL based suites.

    This provides an is_skip() method which will skip the test if any of its
    requirements are not met.

    This is a wrapper around the FastSkip object which makes it easier to
    integrate into existing classes (and maintains API compatibility). It thus
    has all of the same requirements as that class.

    It also provides new attributes:
    gl_required     -- This is a set of extensions that are required for
                       running the test.
    gl_version      -- A float that is the required version number for an
                       OpenGL test.
    gles_version    -- A float that is the required version number for an
                       OpenGL ES test
    glsl_version    -- A float that is the required version number of OpenGL
                       Shader Language for a test
    glsl_es_version -- A float that is the required version number of OpenGL ES
                       Shader Language for a test
    """

    def __init__(self, command, gl_required=None, gl_version=None,
                 gles_version=None, glsl_version=None, glsl_es_version=None,
                 **kwargs):  # pylint: disable=too-many-arguments
        super(FastSkipMixin, self).__init__(command, **kwargs)
        self.__skiper = FastSkip(gl_required=gl_required,
                                 gl_version=gl_version,
                                 gles_version=gles_version,
                                 glsl_version=glsl_version,
                                 glsl_es_version=glsl_es_version)

    @property
    def gl_required(self):
        return self.__skiper.gl_required

    @gl_required.setter
    def gl_required(self, new):
        self.__skiper.gl_required = new

    @property
    def gl_version(self):
        return self.__skiper.gl_version

    @gl_version.setter
    def gl_version(self, new):
        self.__skiper.gl_version = new

    @property
    def gles_version(self):
        return self.__skiper.gles_version

    @gles_version.setter
    def gles_version(self, new):
        self.__skiper.gles_version = new

    @property
    def glsl_version(self):
        return self.__skiper.glsl_version

    @glsl_version.setter
    def glsl_version(self, new):
        self.__skiper.glsl_version = new

    @property
    def glsl_es_version(self):
        return self.__skiper.glsl_es_version

    @glsl_es_version.setter
    def glsl_es_version(self, new):
        self.__skiper.glsl_es_version = new

    def is_skip(self):
        """Skip this test if any of its feature requirements are unmet.

        If no extensions were calculated (if wflinfo isn't installed) then run
        all tests.
        """
        self.__skiper.test()

        super(FastSkipMixin, self).is_skip()


class FastSkipDisabled(object):
    """A no-op version of FastSkip."""

    __slots__ = ['gl_required', 'gl_version', 'gles_version', 'glsl_version',
                 'glsl_es_version']

    def __init__(self, gl_required=None, gl_version=None, gles_version=None,
                 glsl_version=None, glsl_es_version=None):
        self.gl_required = gl_required or set()
        self.gl_version = gl_version
        self.gles_version = gles_version
        self.glsl_version = glsl_version
        self.glsl_es_version = glsl_es_version

    def test(self):
        pass


class FastSkipMixinDisabled(object):
    def __init__(self, command, gl_required=None, gl_version=None,
                 gles_version=None, glsl_version=None, glsl_es_version=None,
                 **kwargs):  # pylint: disable=too-many-arguments
        # Tests that implement the FastSkipMixin expect to have these values
        # set, so just fill them in with the default values.
        self.gl_required = set()
        self.gl_version = None
        self.gles_version = None
        self.glsl_version = None
        self.glsl_es_version = None

        super(FastSkipMixinDisabled, self).__init__(command, **kwargs)


# Shadow the real FastSkipMixin with the Disabled version if
# PIGLIT_NO_FAST_SKIP is truthy
if bool(os.environ.get('PIGLIT_NO_FAST_SKIP', False)):
    warnings.warn('Fast Skipping Disabled')
    # TODO: we can probably get rid of the FastSkipMixinDisabled and just rely
    # on the FastSkipDisabled
    # pylint: disable=invalid-name
    FastSkipMixin = FastSkipMixinDisabled
    FastSkip = FastSkipDisabled
