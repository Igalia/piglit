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
