# coding=utf-8
#
# Copyright © 2016 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

"""Generate fp64 vertex shader input tests."""

from __future__ import print_function, division, absolute_import
import abc
import argparse
import itertools
import os
import types

from six.moves import range

from templates import template_dir
from modules import utils
from modules import types as glsltypes

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

# Hard limit so we don't generate useless tests that cannot be run in any existing HW.
MAX_VERTEX_ATTRIBS = 32

# pylint: disable=bad-whitespace,line-too-long
DOUBLE_INFS            = ['0xfff0000000000000', # -inf
                          '0x7ff0000000000000'] # +inf

DOUBLE_NEG_ZERO        = ['0x8000000000000000'] # Negative underflow (-0.0)

DOUBLE_POS_ZERO        = ['0x0000000000000000'] # Positive underflow (+0.0)

# Double values causing an underflow to zero in any other type
DOUBLE_DENORMAL_VALUES = ['0x800fffffffffffff', # Negative maximum denormalized -- Denormalized may be flushed to 0
                          '0x8000000000000001', # Negative minimum denormalized -- Denormalized may be flushed to 0
                          '0x0000000000000001', # Positive minimum denormalized -- Denormalized may be flushed to 0
                          '0x000fffffffffffff'] # Positive maximum denormalized -- Denormalized may be flushed to 0

DOUBLE_NORMAL_VALUES   = ['0xffefffffffffffff', # Negative maximum normalized
                          '0xcb1e35ed24eb6496', # -7.23401345e+53
                          '0xc8b1381a93a87849', # -1.5e+42
                          '0xc7efffffefffffff', # Negative maximum float normalized
                          '0xc170000000000000', # -16777216.0
                          '0xc014000000000000', # -5.0
                          '0xbfff25ce60000000', # -1.9467300176620483
                          '0x8010000000000000', # Negative minimum normalized
                          '0x0010000000000000', # Positive minimum normalized
                          '0x3fff25ce60000000', # +1.9467300176620483
                          '0x4014000000000000', # +5.0
                          '0x4170000000000000', # +16777216.0
                          '0x47efffffefffffff', # Positive maximum float normalized
                          '0x48b1381a93a87849', # +1.5e+42
                          '0x4b1e35ed24eb6496', # +7.23401345e+53
                          '0x7fefffffffffffff'] # Positive maximum normalized

DSCALAR_TYPES          = [glsltypes.DOUBLE]

DVEC_TYPES             = [glsltypes.DVEC2, glsltypes.DVEC3, glsltypes.DVEC4]

DMAT_TYPES             = [glsltypes.DMAT2, glsltypes.DMAT2X3, glsltypes.DMAT2X4,
                          glsltypes.DMAT3X2, glsltypes.DMAT3, glsltypes.DMAT3X4,
                          glsltypes.DMAT4X2, glsltypes.DMAT4X3, glsltypes.DMAT4]

FSCALAR_TYPES          = [glsltypes.FLOAT]

FVEC_TYPES             = [glsltypes.VEC2, glsltypes.VEC3, glsltypes.VEC4]

FMAT_TYPES             = [glsltypes.MAT2, glsltypes.MAT2X3, glsltypes.MAT2X4,
                          glsltypes.MAT3X2, glsltypes.MAT3, glsltypes.MAT3X4,
                          glsltypes.MAT4X2, glsltypes.MAT4X3, glsltypes.MAT4]

ISCALAR_TYPES          = [glsltypes.INT]

IVEC_TYPES             = [glsltypes.IVEC2, glsltypes.IVEC3, glsltypes.IVEC4]

USCALAR_TYPES          = [glsltypes.UINT]

UVEC_TYPES             = [glsltypes.UVEC2, glsltypes.UVEC3, glsltypes.UVEC4]

HEX_VALUES_32BIT       = ['0xc21620c5', # -3.7532        float, -1038737211 int, 3256230085 uint
                          '0x75bc289b', #  4.7703e32     float,  1975265435 int, 1975265435 uint
                          '0x54c1c081', #  6.6572e12     float,  1421983873 int, 1421983873 uint
                          '0x878218f8', # -1.9575e-34    float, -1038737211 int, 2273450232 uint
                          '0x7e0857ed', #  4.5307886e+37 float,  2114476013 int, 2114476013 uint
                          '0x2bb561bf', #  1.2887954e-12 float,   733307327 int,  733307327 uint
                          '0xff7fffff', # -3.4028235e+38 float,    -8388609 int, 4286578687 uint
                          '0xcb800000', # -16777216.0    float,  -880803840 int, 3414163456 uint
                          '0xc0a00000', # -5.0           float, -1063256064 int, 3231711232 uint
                          '0xbff92e73', # -1.9467300     float, -1074188685 int, 3220778611 uint
                          '0x80800000', # -1.1754944e-38 float, -2139095040 int, 2155872256 uint
                          '0x00000000', #  0.0           float,           0 int,          0 uint
                          '0x00800000', #  1.1754944e-38 float,     8388608 int,    8388608 uint
                          '0x3ff92e73', #  1.9467300     float,  1073294963 int, 1073294963 uint
                          '0x40a00000', #  5.0           float,  1084227584 int, 1084227584 uint
                          '0x4b800000', #  16777216.0    float,  1266679808 int, 1266679808 uint
                          '0x7f7fffff'] #  3.4028235e+38 float,  2139095039 int, 2139095039 uint


# pylint: enable=bad-whitespace,line-too-long


class TestTuple(object):
    """A float64 derived and other type derived tuple to generate the
       needed conversion tests.
    """
    @staticmethod
    def get_dir_name(ver):
        """Returns the directory name to save tests given a GLSL version."""

        assert isinstance(ver, str)
        if ver.startswith('GL_'):
            feature_dir = ver[3:].lower()
        else:
            feature_dir = 'glsl-{}.{}'.format(ver[0], ver[1:])

        return os.path.join('spec', feature_dir, 'execution',
                            'vs_in')

    def __init__(self, ver, names_only):
        assert isinstance(ver, str)
        assert isinstance(names_only, bool)

        self._ver = ver
        self._names_only = names_only

    @abc.abstractmethod
    def generate(self):
        """Generate the GLSL parser tests."""


class RegularTestTuple(TestTuple):
    """Derived class for conversion tests using regular values within the
       edges of the used types.
    """

    @staticmethod
    def create_in_types_array(*types_arrays):
        """Creates vertex input combinations."""
        for product_item in itertools.product(*types_arrays):
            yield product_item

    @staticmethod
    def create_tests(glsl_vers, in_types_array, position_orders, arrays_array, names_only):
        """Creates combinations for flat qualifier tests."""

        assert isinstance(glsl_vers, list)
        assert isinstance(in_types_array, types.GeneratorType)
        assert isinstance(position_orders, list)
        assert isinstance(arrays_array, list)
        assert isinstance(names_only, bool)

        if not names_only:
            for ver in glsl_vers:
                utils.safe_makedirs(TestTuple.get_dir_name(ver))

        for in_types, position_order, arrays, ver in itertools.product(
                in_types_array,
                position_orders,
                arrays_array,
                glsl_vers):
            num_vs_in = 1 # We use an additional vec3 piglit_vertex input
            for idx, in_type in enumerate(in_types):
                num_vs_in += (in_type.columns or 1) * arrays[idx] * \
                    (2 if in_type.type.name == 'double' and in_type.rows in [3, 4] else 1)
                # dvec* and dmat* didn't appear in GLSL until 4.20
                if (in_type.type.name == 'double' and not in_type.scalar) and ver == '410':
                    ver = '420'
            # Skip the test if it needs too many inputs
            if num_vs_in > MAX_VERTEX_ATTRIBS:
                continue

            yield ver, in_types, position_order, arrays, num_vs_in, names_only

    @staticmethod
    def all_tests(names_only):
        """Creates all the combinations for flat qualifier tests."""

        assert isinstance(names_only, bool)

        # We need additional directories for GLSL 420
        if not names_only:
            utils.safe_makedirs(TestTuple.get_dir_name('420'))
        for test_args in RegularTestTuple.create_tests(
                ['GL_ARB_vertex_attrib_64bit', '410'],
                RegularTestTuple.create_in_types_array(
                    itertools.chain(DSCALAR_TYPES, DVEC_TYPES, DMAT_TYPES),
                    itertools.chain(FSCALAR_TYPES, FVEC_TYPES, FMAT_TYPES,
                                    ISCALAR_TYPES, IVEC_TYPES,
                                    USCALAR_TYPES, UVEC_TYPES)),
                [1, 2, 3],
                [[1, 1], [1, 3], [5, 1], [5, 3]],
                names_only):
            yield RegularTestTuple(*test_args)
        for test_args in RegularTestTuple.create_tests(
                ['GL_ARB_vertex_attrib_64bit', '410'],
                RegularTestTuple.create_in_types_array(
                    itertools.chain(FSCALAR_TYPES, FVEC_TYPES, FMAT_TYPES,
                                    ISCALAR_TYPES, IVEC_TYPES,
                                    USCALAR_TYPES, UVEC_TYPES),
                    itertools.chain(DSCALAR_TYPES, DVEC_TYPES, DMAT_TYPES)),
                [1, 2, 3],
                [[1, 1], [1, 2], [3, 1], [3, 2]],
                names_only):
            yield RegularTestTuple(*test_args)
        for test_args in RegularTestTuple.create_tests(
                ['GL_ARB_vertex_attrib_64bit', '410'],
                RegularTestTuple.create_in_types_array(
                    itertools.chain(DSCALAR_TYPES, DVEC_TYPES, DMAT_TYPES),
                    itertools.chain(DSCALAR_TYPES, DVEC_TYPES, DMAT_TYPES)),
                [1, 2, 3],
                [[1, 1], [1, 2], [3, 1], [3, 2]],
                names_only):
            yield RegularTestTuple(*test_args)
        for test_args in RegularTestTuple.create_tests(
                ['GL_ARB_vertex_attrib_64bit', '410'],
                RegularTestTuple.create_in_types_array(
                    itertools.chain(DSCALAR_TYPES, DVEC_TYPES, DMAT_TYPES)),
                [1, 2],
                [[1], [5]],
                names_only):
            yield RegularTestTuple(*test_args)

    def __init__(self, ver, in_types, position_order, arrays, num_vs_in, names_only):
        assert ver in ('GL_ARB_vertex_attrib_64bit', '410', '420')
        assert isinstance(in_types, tuple)
        assert isinstance(position_order, int)
        assert (position_order > 0) and (position_order - 1 <= len(in_types))
        assert isinstance(arrays, list)
        assert isinstance(num_vs_in, int) and (num_vs_in <= MAX_VERTEX_ATTRIBS)
        super(RegularTestTuple, self).__init__(ver, names_only)

        self._in_types = in_types
        self._position_order = position_order
        self._arrays = arrays
        self._num_vs_in = num_vs_in

    def generate(self):
        """Generate GLSL parser tests."""
        filename = os.path.join(TestTuple.get_dir_name(self._ver), 'vs-input')
        for idx, in_type in enumerate(self._in_types):
            if idx == self._position_order - 1:
                filename += '-position'
            filename += '-{}{}'.format(
                in_type.name, '-array{}'.format(
                    self._arrays[idx]) if self._arrays[idx] - 1 else '')
        if self._position_order > len(self._in_types):
            filename += '-position'
        filename += '.shader_test'

        if not self._names_only:
            with open(filename, 'w') as test_file:
                test_file.write(TEMPLATES.get_template(
                    'regular.shader_test.mako').render_unicode(
                        ver=self._ver,
                        in_types=self._in_types,
                        position_order=self._position_order,
                        arrays=self._arrays,
                        num_vs_in=self._num_vs_in,
                        dvalues=DOUBLE_NORMAL_VALUES + DOUBLE_POS_ZERO,
                        hvalues=HEX_VALUES_32BIT))
        print(filename)


class ColumnsTestTuple(TestTuple):
    """Derived class for conversion tests using regular values within the
       edges of the used types.
    """

    @staticmethod
    def all_tests(names_only):
        """Creates all the combinations for flat qualifier tests."""

        assert isinstance(names_only, bool)
        glsl_vers = ['GL_ARB_vertex_attrib_64bit', '420']

        if not names_only:
            for ver in glsl_vers:
                utils.safe_makedirs(TestTuple.get_dir_name(ver))

        for mat in DMAT_TYPES:
            for columns in itertools.product(range(2), repeat=mat.columns):
                if (0 not in columns) or (1 not in columns):
                    continue
                for ver in glsl_vers:
                    yield ColumnsTestTuple(ver, mat, columns, names_only)

    def __init__(self, ver, mat, columns, names_only):
        assert ver in ('GL_ARB_vertex_attrib_64bit', '420')
        super(ColumnsTestTuple, self).__init__(ver, names_only)

        self._mat = mat
        self._columns = columns

    def generate(self):
        """Generate GLSL parser tests."""

        filename = os.path.join(TestTuple.get_dir_name(self._ver),
                                'vs-input-columns-{}'.format(self._mat.name))
        for idx, column in enumerate(self._columns):
            if column == 1:
                filename += '-{}'.format(idx)
        filename += '.shader_test'

        if not self._names_only:
            with open(filename, 'w') as test_file:
                test_file.write(TEMPLATES.get_template(
                    'columns.shader_test.mako').render_unicode(
                        ver=self._ver,
                        mat=self._mat,
                        columns=self._columns,
                        dvalues=DOUBLE_NORMAL_VALUES + DOUBLE_POS_ZERO))
        print(filename)


def main():
    """Main function."""

    parser = argparse.ArgumentParser(
        description="Generate non-flat interpolation qualifier tests with fp64 types")
    parser.add_argument(
        '--names-only',
        dest='names_only',
        action='store_true',
        default=False,
        help="Don't output files, just generate a list of filenames to stdout")
    args = parser.parse_args()

    for test in itertools.chain(RegularTestTuple.all_tests(args.names_only),
                                ColumnsTestTuple.all_tests(args.names_only)):
        test.generate()


if __name__ == '__main__':
    main()
