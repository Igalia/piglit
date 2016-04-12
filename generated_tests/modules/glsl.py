# encoding=utf-8
# Copyright © 2016 Intel Corporation

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

"""Provides helper classes for representing glsl data."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import functools

import six

import compat

__all__ = [
    'GLSLESVersion',
    'GLSLVersion',
    'Version',
]


class _Version(object):
    """Factory class for glsl versions.

    provides an object cache to reduce duplication of objects. This object
    should not be initialized more than once, to avoid that use the Version
    constant provided by the module.

    It would provide either a GLSLVersion or a GLSLESVersion.

    """
    _es_versions = [
        '100',
        '300 es',
        '310 es',
        '320 es',
    ]

    _versions = [
        '110',
        '120',
        '130',
        '140',
        '150',
        '330',
        '400',
        '410',
        '420',
        '430',
        '440',
        '450',
    ]

    def __init__(self):
        self.__cache = {}

    def __call__(self, ver):
        """Make a Version object, or provide one from the cache."""
        assert isinstance(ver, six.text_type)

        # Try to get an object from the cache, if that fails create a new one
        # and add it to the cache before returning it.
        try:
            return self.__cache[ver]
        except KeyError:
            if ver in self._es_versions:
                obj = GLSLESVersion(ver)
            elif ver in self._versions:
                obj = GLSLVersion(ver)
            else:
                raise Exception('Undefined version {}'.format(ver))

            self.__cache[ver] = obj
            return obj


Version = _Version()  # pylint: disable=invalid-name


@compat.python_2_unicode_compatible  # pylint: disable=no-member
@functools.total_ordering
class GLSLVersion(object):
    """A Representation of an OpenGL Shading Language version.

    This object provides a bunch of the niceties that one would want. It's
    orderable (can be sorted, and can be compared with the standard ==, !=, <,
    etc), can be called with str() (which provides the integer version, like
    120), and a method to print the decimal version (1.20).

    Do not initialize this directly.

    """
    def __init__(self, ver):
        assert ver in ['110', '120', '130', '140', '150', '330', '400', '410',
                       '420', '430', '440', '450']
        self._internal = ver
        self.is_es = False

    def __str__(self):
        return self._internal

    def __int__(self):
        return int(self._internal)

    def __float__(self):
        return float(int(self) / 100)

    def __eq__(self, other):
        # If the other version is ES then we know they're not equal
        if isinstance(other, GLSLESVersion):
            return False
        elif isinstance(other, (GLSLVersion, int)):
            return int(self) == int(other)
        elif isinstance(other, float):
            return float(self) == float(other)
        else:
            return NotImplemented

    def __ne__(self, other):
        return not self == other

    def __lt__(self, other):
        if isinstance(other, GLSLESVersion):
            raise TypeError('Unorderable types GLSLVersion and GLSLESVersion')
        elif isinstance(other, (GLSLVersion, int)):
            return int(self) < int(other)
        elif isinstance(other, float):
            return float(self) < other
        else:
            return NotImplemented

    def print_float(self):
        """A version suitable to print as a decimal with two trailing digits."""
        return '{:.2f}'.format(float(self))


@compat.python_2_unicode_compatible  # pylint: disable=no-member
@functools.total_ordering
class GLSLESVersion(object):
    """Represents a GLSL ES version.

    Do not initialize this directly.

    """
    def __init__(self, ver):
        if ver == '100':
            self._internal = ver
        else:
            self._internal = ver[:-3]  # drop " es"
        self.is_es = True

    def __str__(self):
        if self._internal == '100':
            return self._internal
        else:
            return self._internal + " es"

    def __int__(self):
        return int(self._internal)

    def __float__(self):
        return float(int(self) / 100)

    def __eq__(self, other):
        # If the other version is ES then we know they're not equal
        if isinstance(other, GLSLVersion):
            return False
        elif isinstance(other, (GLSLESVersion, int)):
            return int(self) == int(other)
        elif isinstance(other, float):
            return float(self) == float(other)
        else:
            return NotImplemented

    def __ne__(self, other):
        return not self == other

    def __lt__(self, other):
        if isinstance(other, GLSLVersion):
            raise TypeError('Unorderable types GLSLESVersion and GLSLVersion')
        elif isinstance(other, (GLSLESVersion, int)):
            return int(self) < int(other)
        elif isinstance(other, float):
            return float(self) < other
        else:
            return NotImplemented

    def print_float(self):
        """A version suitable to print as a decimal with two trailing digits."""
        if self._internal == '100':
            return '{:.2f}'.format(float(self))
        else:
            return '{:.2f} es'.format(float(self))


class _MinVersion(object):
    """A factory class for sorting GLSL and GLSLES versions.

    This stores the minimum version required for various operations (currently
    only for_stage and for_stage_with_ext).

    This class is not meant to be reinitialized, instead use the provided
    MinVersion constant.

    """
    __gl_stage_min = {
        'frag': Version('110'),
        'vert': Version('110'),
        'geom': Version('150'),
        'tesc': Version('400'),
        'tese': Version('400'),
        'comp': Version('430'),
    }
    # Only versions that actaly change are here, the function will return the
    # values from __gl_stage_min if they're not here
    __gl_stage_min_ext = {
        # geometry_shader4 is not included here intentionally. It is
        # significantly different than the geometry shaders in OpenGL 3.2
        'tesc': (Version('140'), 'GL_ARB_tesselation_shader'),
        'tese': (Version('140'), 'GL_ARB_tesselation_shader'),
        'comp': (Version('140'), 'GL_ARB_compute_shader'),
    }
    __gles_stage_min = {
        'frag': Version('100'),
        'vert': Version('100'),
        'comp': Version('310 es'),
        'geom': Version('320 es'),
        'tesc': Version('320 es'),
        'tese': Version('320 es'),
    }
    # Only versions that actaly change are here, the function will return the
    # values from __gles_stage_min if they're not here
    __gles_stage_min_ext = {
        'geom': (Version('310 es'), 'GL_OES_geometry_shader'),
        'tesc': (Version('310 es'), 'GL_OES_tesselation_shader'),
        'tese': (Version('310 es'), 'GL_OES_tesselation_shader'),
    }

    def for_stage(self, stage, version):
        """Return max(stage minimum version, requested version).

        When provided a stage and a version, it will return the greater of the
        provided version and the minimum version of that stage without an
        extension. For example, in OpenGL teselation is available in GLSL
        4.00+, or in 1.40+ with ARB_tesselation_shader. Given Version('150')
        and 'tesc' this method returns Version('400').

        Arguments:
        stage -- A stage named by the extensions glslparsertest uses (frag,
                 vert, geom, tesc, tese, comp)
        version -- A version as returned by the Version function.

        >>> m = _MinVersion()
        >>> m.for_stage('geom', Version('300 es'))
        Version('320 es')
        >>> m.for_stage('frag', Version('130'))
        Version('130')

        """
        assert isinstance(version, (GLSLVersion, GLSLESVersion))
        if isinstance(version, GLSLVersion):
            _stage = self.__gl_stage_min[stage]
        elif isinstance(version, GLSLESVersion):
            _stage = self.__gles_stage_min[stage]

        return _stage if _stage > version else version

    def for_stage_with_ext(self, stage, version):
        """Return the earliest GLSL version that a stage is supported in with
        an extension.

        When provided a stage and a version, it will return the greater of the
        provided version and the minimum version of that stage with an
        extension, and if necissary the extension as a string. For example, in
        OpenGL teselation is available in GLSL 4.00+, or in 1.40+ with
        ARB_tesselation_shader. Given Version('150') and 'tesc' this method
        returns (Version('150'), 'GL_ARB_tesselation_shader'); but given
        Version('400') and 'tesc' it returns (Version('400'), None)

        If there is no extension (like with fragment and vertex) then None will
        be returned as the secon value. It is up to the caller to handle this
        appropriately. It will also return None for the extension when the GLSL
        version is high enough to not require an extension.

        Takes the same arguments as for_stage.

        >>> m = _MinVersion()
        >>> m.for_stage_with_ext('geom', Version('300 es'))
        (Version('310 es'), 'GL_OES_geometry_shader')
        >>> m.for_stage_with_ext('frag', Version('130'))
        (Version('130'), None)

        """
        assert isinstance(version, (GLSLVersion, GLSLESVersion))
        if isinstance(version, GLSLVersion):
            try:
                _stage, ext = self.__gl_stage_min_ext[stage]
            except KeyError:
                _stage, ext = self.__gl_stage_min[stage], None
        elif isinstance(version, GLSLESVersion):
            try:
                _stage, ext = self.__gles_stage_min_ext[stage]
            except KeyError:
                _stage, ext = self.__gles_stage_min[stage], None

        # If the version queried is less than the required, return the require
        # and the ext
        if _stage > version:
            return (_stage, ext)
        # If the requested version is greater or equal to the version that the
        # feature became core in, return the version and None
        elif self.for_stage(stage, version) <= version:
            return (version, None)
        # Otherwise the requested version and the extension are returned
        else:
            return (version, ext)


MinVersion = _MinVersion()  # pylint: disable=invalid-name
