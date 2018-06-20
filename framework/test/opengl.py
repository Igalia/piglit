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
    api            -- The API required.
                      One of [gles1, gles2, gles3, core, compat]
    extensions     -- A set of extensions required
    api_version    -- The version of the API required
    shader_version -- The versoin of the shader language required
    """
    __slots__ = ['api', 'extensions', 'api_version', 'shader_version']

    info = wflinfo.WflInfo()

    def __init__(self, api=None, extensions=None, api_version=None,
                 shader_version=None):
        self.extensions = extensions or set()
        self.api = api
        self.api_version = api_version
        self.shader_version = shader_version

    def test(self):
        """Skip this test if any of its feature requirements are unmet.

        If no extensions were calculated (if wflinfo isn't installed) then run
        all tests.

        Raises:
        TestIsSkip   -- if any of the conditions passed to self are false
        """
        if not self.api:
            check = self.info.compat
        elif self.api in ['gles2', 'gles3']:
            check = self.info.es2
        elif self.api == 'gles1':
            check = self.info.es1
        else:
            check = getattr(self.info, self.api)

        if check.extensions:
            for extension in self.extensions:
                if extension not in check.extensions:
                    raise TestIsSkip(
                        'Test requires extension {} '
                        'which is not available'.format(extension))

        # TODO: Be able to handle any operator
        if (check.api_version is not None
                and self.api_version is not None
                and self.api_version > check.api_version):
            raise TestIsSkip(
                'Test requires OpenGL {} version {}, '
                'but only {} is available'.format(
                    self.api, self.api_version, check.api_version))

        # TODO: Be able to handle any operator
        if (check.shader_version is not None
                and self.shader_version is not None
                and self.shader_version > check.shader_version):
            raise TestIsSkip(
                'Test requires OpenGL {} Shader Language version {}, '
                'but only {} is available'.format(
                    self.api, self.shader_version, check.shader_version))


class FastSkipMixin(object):
    """Fast test skipping for OpenGL based suites.

    This provides an is_skip() method which will skip the test if any of its
    requirements are not met.

    This is a wrapper around the FastSkip object which makes it easier to
    integrate into existing classes (and maintains API compatibility). It thus
    has all of the same requirements as that class.

    It also provides new attributes:
    require_extensions -- A set of extensions that are requuired for running
                          this test.
    require_shader    -- The shader language version required.
    reqiure_version   -- The API version required.
    require_api       -- The API required.
    """

    def __init__(self, command, api=None, extensions=None, api_version=None,
                 shader_version=None, **kwargs):
        super(FastSkipMixin, self).__init__(command, **kwargs)
        self.__skiper = FastSkip(api=api,
                                 extensions=extensions,
                                 api_version=api_version,
                                 shader_version=shader_version)

    @property
    def require_extensions(self):
        return self.__skiper.extensions

    @property
    def require_api(self):
        return self.__skiper.api

    @property
    def require_shader(self):
        return self.__skiper.shader_version

    @property
    def require_version(self):
        return self.__skiper.api_version

    def is_skip(self):
        """Skip this test if any of its feature requirements are unmet.

        If no extensions were calculated (if wflinfo isn't installed) then run
        all tests.
        """
        self.__skiper.test()

        super(FastSkipMixin, self).is_skip()


class FastSkipDisabled(object):
    """A no-op version of FastSkip."""

    __slots__ = ['api', 'extensions', 'api_version', 'shader_version']

    info = wflinfo.WflInfo()

    def __init__(self, api=None, extensions=None, api_version=None,
                 shader_version=None):
        self.extensions = set()
        self.api = api
        self.api_version = api_version
        self.shader_version = shader_version

    def test(self):
        pass


class FastSkipMixinDisabled(object):
    def __init__(self, command, api=None, extensions=None, api_version=None,
                 shader_version=None, **kwargs):
        # Tests that implement the FastSkipMixin expect to have these values
        # set, so just fill them in with the default values.
        self.require_extensions = set()
        self.require_shader = None
        self.reqiure_version = None
        self.require_api = None

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
