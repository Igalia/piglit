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

"""Classes describing various GLSL types.

These classes do not contain values themselves, rather they describe the GLSL
types.

Included are representations of Matrices, Vectors, and Scalars as containers,
and Ints, Uints, Floats, and Doubles as types. This can easily be expanded to
include more types, and the corresponding containers.

Although the classes that implement these representations are not marked as
private, most consumers will be happier using the provided constants than using
the classes themselves. Probably the only real use for the types is to do type
checking.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import six

from .utils import lazy_property


class Container(object):
    """A Base class for GLSL container.

    It provides the following attributes:
    name    -- The string formatted name of the container
    scalar  -- True if the type is scalar
    vector  -- True if the type is vector
    matrix  -- True if the type is matrix
    type    -- A Type object that is the type contained by the container
    rows    -- The number of rows the container has. Scalars return 1
    columns -- The number of rows container has. Scalars and Vectors return 1

    """
    def __init__(self, name, is_scalar=False, is_vector=False, is_matrix=False,
                 rows=1, columns=1, contains=None):
        assert all(isinstance(x, bool)
                   for x in [is_scalar, is_vector, is_matrix]), \
            'is_scalar, is_vector, and is_matrix must be bools'
        assert [is_scalar, is_vector, is_matrix].count(True) == 1, \
            'A type must be exactly one of: scalar, vector, matrix'
        assert isinstance(contains, Type), 'contains must be a type instance'
        assert isinstance(name, six.text_type), 'name must be string'
        assert columns is None or isinstance(columns, int), \
            'columns must be an int if provided'
        assert rows is None or isinstance(rows, int), \
            'row must be an int if provided'

        self.name = name
        self.scalar = is_scalar
        self.vector = is_vector
        self.matrix = is_matrix
        self.type = contains
        self.rows = rows
        self.columns = columns


class Scalar(Container):
    """A base class for Scalar types."""
    def __init__(self, name, contains):
        super(Scalar, self).__init__(name, is_scalar=True, contains=contains)


class Vector(Container):
    """A base class for Vector Containers."""
    def __init__(self, name, rows, contains):
        assert isinstance(rows, int), 'rows must be an integer'
        assert rows in [2, 3, 4], 'vecs can only be 2-4 in GLSL'

        super(Vector, self).__init__(name, is_vector=True, rows=rows,
                                     contains=contains)


class Matrix(Container):
    """A base class for vectory Containers."""
    def __init__(self, name, columns, rows, contains):
        assert isinstance(rows, int), 'rows must be an integer'
        assert isinstance(columns, int), 'columns must be an integer'
        assert rows in [2, 3, 4], 'Matrix rows can only be 2-4 in GLSL'
        assert columns in [2, 3, 4], 'Matrix columns can only be 2-4 in GLSL'

        super(Matrix, self).__init__(name, is_matrix=True, rows=rows,
                                     columns=columns, contains=contains)

    @lazy_property
    def square(self):
        return self.columns == self.rows


class Type(object):
    """Class representing a GLSL type.

    Provides the following interfaces currently:
    integer -- True if the type is an integer type
    signed  -- True if the type is a signed type
    float   -- True if the type is a floating point type
    size    -- The integer size of the type (8, 16, 32, 64, 128, etc)
    name    -- A formated string name of the type. (float, double, int64, ...)

    """
    def __init__(self, name, integer=False, signed=False, floating=False,
                 size=None):
        assert [integer, floating].count(True) == 1, \
            'Type can onnly be float or int'
        assert integer or (float and signed), 'Floats cannot be unsigned'
        assert isinstance(size, int), 'size must be an int'

        self.name = name
        self.integer = integer
        self.signed = signed
        self.float = floating
        self.size = size


# pylint: disable=bad-whitespace
# Type definitions, these are used internally by the Scalar/Vector/Matrix
# constructs
INT_TYPE    = Type('int',    integer=True,  signed=True,  size=32)
UINT_TYPE   = Type('uint',   integer=True,  signed=False, size=32)
FLOAT_TYPE  = Type('float',  floating=True, signed=True,  size=32)
DOUBLE_TYPE = Type('double', floating=True, signed=True,  size=64)

# Container definitions
INT     = Scalar('int',    INT_TYPE)
UINT    = Scalar('uint',   UINT_TYPE)
FLOAT   = Scalar('float',  FLOAT_TYPE)
DOUBLE  = Scalar('double', DOUBLE_TYPE)

VEC2    = Vector('vec2',  2, FLOAT_TYPE)
VEC3    = Vector('vec3',  3, FLOAT_TYPE)
VEC4    = Vector('vec4',  4, FLOAT_TYPE)

IVEC2   = Vector('ivec2', 2, INT_TYPE)
IVEC3   = Vector('ivec3', 3, INT_TYPE)
IVEC4   = Vector('ivec4', 4, INT_TYPE)

UVEC2   = Vector('uvec2', 2, UINT_TYPE)
UVEC3   = Vector('uvec3', 3, UINT_TYPE)
UVEC4   = Vector('uvec4', 4, UINT_TYPE)

DVEC2   = Vector('dvec2', 2, DOUBLE_TYPE)
DVEC3   = Vector('dvec3', 3, DOUBLE_TYPE)
DVEC4   = Vector('dvec4', 4, DOUBLE_TYPE)

MAT2    = Matrix('mat2',    2, 2, FLOAT_TYPE)
MAT3    = Matrix('mat3',    3, 3, FLOAT_TYPE)
MAT4    = Matrix('mat4',    4, 4, FLOAT_TYPE)
MAT2X2  = Matrix('mat2x2',  2, 2, FLOAT_TYPE)
MAT2X3  = Matrix('mat2x3',  2, 3, FLOAT_TYPE)
MAT2X4  = Matrix('mat2x4',  2, 4, FLOAT_TYPE)
MAT3X2  = Matrix('mat3x2',  3, 2, FLOAT_TYPE)
MAT3X3  = Matrix('mat3x3',  3, 3, FLOAT_TYPE)
MAT3X4  = Matrix('mat3x4',  3, 4, FLOAT_TYPE)
MAT4X2  = Matrix('mat4x2',  4, 2, FLOAT_TYPE)
MAT4X3  = Matrix('mat4x3',  4, 3, FLOAT_TYPE)
MAT4X4  = Matrix('mat4x4',  4, 4, FLOAT_TYPE)

DMAT2   = Matrix('dmat2',   2, 2, DOUBLE_TYPE)
DMAT3   = Matrix('dmat3',   3, 3, DOUBLE_TYPE)
DMAT4   = Matrix('dmat4',   4, 4, DOUBLE_TYPE)
DMAT2X2 = Matrix('dmat2x2', 2, 2, DOUBLE_TYPE)
DMAT2X3 = Matrix('dmat2x3', 2, 3, DOUBLE_TYPE)
DMAT2X4 = Matrix('dmat2x4', 2, 4, DOUBLE_TYPE)
DMAT3X2 = Matrix('dmat3x2', 3, 2, DOUBLE_TYPE)
DMAT3X3 = Matrix('dmat3x3', 3, 3, DOUBLE_TYPE)
DMAT3X4 = Matrix('dmat3x4', 3, 4, DOUBLE_TYPE)
DMAT4X2 = Matrix('dmat4x2', 4, 2, DOUBLE_TYPE)
DMAT4X3 = Matrix('dmat4x3', 4, 3, DOUBLE_TYPE)
DMAT4X4 = Matrix('dmat4x4', 4, 4, DOUBLE_TYPE)
# pylint: enable=bad-whitespace
