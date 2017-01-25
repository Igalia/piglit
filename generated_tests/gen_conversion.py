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

"""Generate fp64 types conversion tests."""

from __future__ import print_function, division, absolute_import
import abc
import argparse
import itertools
import os
import struct

import numpy as np

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

# pylint: disable=bad-whitespace,line-too-long,bad-continuation
DOUBLE_INFS                 = ['0xfff0000000000000', # -inf
                               '0x7ff0000000000000'] # +inf

DOUBLE_NEG_ZERO             = ['0x8000000000000000'] # Negative underflow (-0.0)

DOUBLE_POS_ZERO             = ['0x0000000000000000'] # Positive underflow (+0.0)

# Double values causing an underflow to zero in any other type
DOUBLE_DENORMAL_VALUES      = ['0x800fffffffffffff', # Negative maximum denormalized -- Denormalized may be flushed to 0
                               '0x8000000000000001', # Negative minimum denormalized -- Denormalized may be flushed to 0
                               '0x0000000000000001', # Positive minimum denormalized -- Denormalized may be flushed to 0
                               '0x000fffffffffffff'] # Positive maximum denormalized -- Denormalized may be flushed to 0

DOUBLE_NORMAL_VALUES        = ['0x8010000000000000', # Negative minimum normalized
                               '0x0010000000000000'] # Positive minimum normalized

# Double +/-inf
DOUBLE_FLOAT_INFS           = ['0xc7effffff0000000', # Negative overflow (-inf)
                               '0x47effffff0000000'] # Positive overflow (+inf)

DOUBLE_FLOAT_VALUES         = ['0xc7efffffefffffff', # Negative maximum normalized
                               '0xc170000000000000', # -16777216.0
                               '0xc014000000000000', # -5.0
                               '0xbfff25ce60000000', # -1.9467300176620483
                               '0xb80fffffe0000000', # Negative minimum normalized
                               '0xb69fffffffffffff', # Negative underflow
                               '0x369fffffffffffff', # Positive underflow
                               '0x380fffffe0000000', # Positive minimum normalized
                               '0x3fff25ce60000000', # +1.9467300176620483
                               '0x4014000000000000', # +5.0
                               '0x4170000000000000', # +16777216.0
                               '0x47efffffefffffff'] # Positive maximum normalized

DOUBLE_UINT_VALUES          = ['0xbfeccccccccccccd', # -0.9
                               #'0x8010000000000000', # Negative minimum normalized          -- Already checked
                               #'0x800fffffffffffff', # Negative maximum denormalized        -- Already checked
                               #'0x8000000000000001', # Negative minimum denormalized        -- Already checked
                               #'0x8000000000000000', # Negative minimum        (-0)         -- Already checked
                               #'0x0000000000000000', # Positive minimum        (+0)         -- Already checked
                               '0x3fff25ce60000000', # +1.9467300176620483
                               '0x4014000000000000', # +5.0
                               '0x4170000000000000', # +16777216.0
                               '0x41dfffffffc00000', # Signed int low frontier (+2147483647)
                               '0x41e0000000000000', # Signed int up frontier  (+2147483648)
                               '0x41efffffffe00000'] # Maximum                 (+4294967295)

DOUBLE_INT_VALUES           = ['0xc1e0000000000000', # Minimum          (-2147483648)
                               '0xc170000000000000', # -16777216.0
                               '0xc014000000000000', # -5.0
                               '0xbfff25ce60000000', # -1.9467300176620483
                               #'0x8000000000000000', # Negative minimum        (-0)         -- Already checked
                               #'0x0000000000000000', # Minimum                 (+0)         -- Already checked
                               '0x3fff25ce60000000', # +1.9467300176620483
                               '0x4014000000000000', # +5.0
                               '0x4170000000000000', # +16777216.0
                               '0x41dfffffffc00000'] # Maximum          (+2147483647)

DOUBLE_BOOL_VALUES          = [#'0x8010000000000000', # Minimum negative True value -- Already checked
                               #'0x0000000000000000', # False                       -- Already checked
                               #'0x0010000000000000', # Minimum positive True value -- Already checked
                              ]

FLOAT_INFS                  = ['0xff800000', # -inf
                               '0x7f800000'] # +inf

FLOAT_NEG_ZERO              = ['0x80000000'] # Negative underflow (-0.0)

FLOAT_POS_ZERO              = ['0x00000000'] # Positive underflow (+0.0)

FLOAT_VALUES                = ['0xff7fffff', # Negative maximum normalized
                               '0xcb800000', # -16777216.0
                               '0xc0a00000', # -5.0
                               '0xbff92e73', # -1.9467300176620483
                               '0x80800000', # Negative minimum normalized
                               #'0x807fffff', # Negative maximum denormalized -- Denormalized may be flushed to 0
                               #'0x80000001', # Negative minimum denormalized -- Denormalized may be flushed to 0
                               #'0x00000001', # Positive minimum denormalized -- Denormalized may be flushed to 0
                               #'0x007fffff', # Positive maximum denormalized -- Denormalized may be flushed to 0
                               '0x00800000', # Positive minimum normalized
                               '0x3ff92e73', # +1.9467300176620483
                               '0x40a00000', # +5.0
                               '0x4b800000', # +16777216.0
                               '0x7f7fffff'] # Positive maximum normalized

UINT_VALUES                 = ['0', # Minimum
                               '5',
                               '2147483647', # Signed int low frontier
                               '2147483648', # Signed int up frontier
                               '4294967295'] # Maximum

INT_VALUES                  = ['-2147483648', # Minimum
                               '-5',
                               '-1',
                               '0',
                               '1',
                               '5',
                               '2147483647'] # Maximum

BOOL_VALUES                 = ['0', # False
                               '1'] # True
# pylint: enable=bad-whitespace,line-too-long,bad-continuation

def get_dir_name(ver, test_type):
    """Returns the directory name to save tests given a GLSL version and a
       test type.
    """

    assert isinstance(ver, str)
    assert isinstance(test_type, str)
    if ver.startswith('GL_'):
        feature_dir = ver[3:].lower()
    else:
        feature_dir = 'glsl-{}.{}'.format(ver[0], ver[1:])

    return os.path.join('spec', feature_dir, test_type,
                        'conversion')


class TestTuple(object):
    """A float64 derived and other type derived tuple to generate the
       needed conversion tests.
    """

    @staticmethod
    def float_to_hex(fvalue):
        """Returns the hexadecimal representation from a float32 value."""
        assert isinstance(fvalue, np.float32)
        return hex(struct.unpack('<I', struct.pack('<f', fvalue))[0])

    @staticmethod
    def double_to_hex(fvalue):
        """Returns the hexadecimal representation from a float64 value."""
        assert isinstance(fvalue, float)
        return hex(struct.unpack('<Q', struct.pack('<d', fvalue))[0]).rstrip("L")

    @staticmethod
    def hex_to_float(hstr):
        """Returns a float32 value from its hexadecimal representation."""
        assert isinstance(hstr, str)
        return struct.unpack('<f', struct.pack('<I', int(hstr, 16)))[0]

    @staticmethod
    def hex_to_double(hstr):
        """Returns a float64 value from its hexadecimal representation."""

        assert isinstance(hstr, str)
        return struct.unpack('<d', struct.pack('<Q', int(hstr, 16)))[0]

    @staticmethod
    def float_hex_to_double_hex(hstr):
        """Returns the float64 hexadecimal representation from a float32
           hexadecimal representation.
        """
        assert isinstance(hstr, str)
        double_value = TestTuple.hex_to_float(hstr)
        return TestTuple.double_to_hex(double_value)

    @staticmethod
    def float_hex_to_inv_double_hex(hstr):
        """Returns the inverted float64 hexadecimal representation from a
           float32 hexadecimal representation.
        """
        assert isinstance(hstr, str)
        temp = TestTuple.hex_to_float(hstr)
        double_value = np.divide(1.0, temp)
        return TestTuple.double_to_hex(double_value)

    @staticmethod
    def int_str_to_double_str(istr):
        """Returns a float64 string from an int32 string."""
        assert isinstance(istr, str)
        return str(float(istr))

    @staticmethod
    def double_hex_to_bool_str(hstr):
        """Returns a bool string from a float64 hexadecimal representation."""
        assert isinstance(hstr, str)
        bool_double = TestTuple.hex_to_double(hstr)
        return '1' if bool_double != 0.0 else '0'

    @staticmethod
    def double_hex_to_int_str(hstr):
        """Returns an int32 string from a float64 hexadecimal
           representation.
        """
        assert isinstance(hstr, str)
        int_double = TestTuple.hex_to_double(hstr)
        if int_double > np.iinfo(np.dtype('int32')).max:
            return str(np.iinfo(np.dtype('int32')).max)
        if int_double < np.iinfo(np.dtype('int32')).min:
            return str(np.iinfo(np.dtype('int32')).min)
        return str(int(int_double))

    @staticmethod
    def double_hex_to_uint_str(hstr):
        """Returns an uint32 string from a float64 hexadecimal
           representation.
        """
        assert isinstance(hstr, str)
        uint_double = TestTuple.hex_to_double(hstr)
        if uint_double > np.iinfo(np.dtype('uint32')).max:
            return str(np.iinfo(np.dtype('uint32')).max)
        if uint_double < np.iinfo(np.dtype('uint32')).min:
            return str(np.iinfo(np.dtype('uint32')).min)
        return str(int(uint_double))

    @staticmethod
    def double_hex_to_float_hex(hstr):
        """Returns the float32 hexadecimal representation from a float64
           hexadecimal representation.
        """
        assert isinstance(hstr, str)
        float_double = np.float32(TestTuple.hex_to_double(hstr))
        return TestTuple.float_to_hex(float_double)

    @staticmethod
    def double_hex_to_inv_float_hex(hstr):
        """Returns the inverted float32 hexadecimal representation from a
           float64 hexadecimal representation.
        """
        assert isinstance(hstr, str)
        temp = np.divide(1.0, TestTuple.hex_to_double(hstr))
        float_double = np.float32(temp)
        return TestTuple.float_to_hex(float_double)

    def __init__(self, ver, stage,
                 first_dimension, second_dimension,
                 basic_type, target_type, names_only):
        assert stage in ('vert', 'geom', 'frag')
        assert first_dimension in ('1', '2', '3', '4')
        assert second_dimension in ('1', '2', '3', '4')
        assert isinstance(names_only, bool)

        self._ver = ver
        self._stage = stage
        self._basic_type = basic_type
        self._target_type = target_type
        self._names_only = names_only
        self._target_full_type = ''
        self._conversion_type = ''
        self._uniform_type = ''
        self._amount = int(first_dimension) * int(second_dimension)
        self._filenames = []
        self._extensions = []

        if ver.startswith('GL_'):
            if basic_type == 'd' or target_type == 'd':
                self._extensions.append('GL_ARB_gpu_shader_fp64')

        if first_dimension != '1':
            dimensional_type = 'mat' + first_dimension
            if first_dimension != second_dimension:
                dimensional_type += 'x' + second_dimension
        elif second_dimension != '1':
            dimensional_type = 'vec' + second_dimension
        else:
            dimensional_type = ''

        if dimensional_type == '':
            if basic_type == 'b':
                self._conversion_type = 'bool'
                self._uniform_type = 'int'
            elif basic_type == 'i':
                self._conversion_type = 'int'
            elif basic_type == 'u':
                self._conversion_type = 'uint'
            elif basic_type == 'f':
                self._conversion_type = 'float'
            self._target_full_type = 'double'
            if self._uniform_type == '':
                self._uniform_type = self._conversion_type
        else:
            self._conversion_type = (basic_type if basic_type != 'f' else '') + dimensional_type
            if basic_type == 'b':
                self._uniform_type = 'i' + dimensional_type
            else:
                self._uniform_type = self._conversion_type
            self._target_full_type = 'd' + dimensional_type

    @abc.abstractmethod
    def _gen_to_target(self):
        """Generates the test files for conversions to float64."""

    @abc.abstractmethod
    def _gen_from_target(self):
        """Generates the test files for conversions from float64."""

    @property
    def filenames(self):
        """Returns the test file names this tuple will generate."""
        if self._filenames == []:
            tmp = self._names_only
            self._names_only = True
            self.generate_test_files()
            self._names_only = tmp
        return self._filenames

    def generate_test_files(self):
        """Generate the GLSL parser tests."""
        self._filenames = []

        self._gen_to_target()
        self._gen_from_target()


class RegularTestTuple(TestTuple):
    """Derived class for conversion tests using regular values within the
       edges of the used types.
    """

    @staticmethod
    def all_tests(names_only):
        """Returns all the possible contained conversion test instances."""

        assert isinstance(names_only, bool)
        stages = ['vert', 'geom', 'frag']
        dimensions = ['1', '2', '3', '4']
        basic_types = ['b', 'u', 'i', 'f']
        target_types = ['d']
        glsl_ver = ['GL_ARB_gpu_shader_fp64', '400']

        if not names_only:
            test_types = ['compiler', 'execution']
            for ver, test_type in itertools.product(glsl_ver, test_types):
                utils.safe_makedirs(get_dir_name(ver, test_type))

        for ver, stage, first_dimension, second_dimension, basic_type, target_type in itertools.product(
                glsl_ver,
                stages,
                dimensions,
                dimensions,
                basic_types,
                target_types):
            if (not (first_dimension != '1' and (second_dimension == '1' or basic_type != 'f')) and
                (basic_type not in target_types or basic_type < target_type)):
                yield RegularTestTuple(ver, stage,
                                       first_dimension, second_dimension,
                                       basic_type, target_type, names_only)

    def __init__(self, ver, stage,
                 first_dimension, second_dimension,
                 basic_type, target_type, names_only):
        assert ver in ('GL_ARB_gpu_shader_fp64', '400')
        assert basic_type in ('b', 'u', 'i', 'f')
        assert target_type in ('d')
        assert not (first_dimension != '1' and (second_dimension == '1' or basic_type != 'f'))
        super(RegularTestTuple, self).__init__(ver, stage,
                                               first_dimension, second_dimension,
                                               basic_type, target_type, names_only)

    def _gen_comp_test(self, from_type, to_type, converted_from):
        filename = os.path.join(
            get_dir_name(self._ver, 'compiler'),
            '{}-conversion-implicit-{}-{}-bad.{}'.format(self._stage, from_type, to_type,
                                                         self._stage))

        self._filenames.append(filename)

        if not self._names_only:
            with open(filename, 'w') as test_file:
                test_file.write(TEMPLATES.get_template(
                    'compiler.{}.mako'.format(self._stage)).render_unicode(
                        ver=self._ver,
                        extensions=self._extensions,
                        from_type=from_type,
                        to_type=to_type,
                        converted_from=converted_from))

    def _gen_exec_test(self, from_type, to_type,
                       uniform_from_type, uniform_to_type,
                       explicit, converted_from, conversions):
        filename = os.path.join(
            get_dir_name(self._ver, 'execution'),
            '{}-conversion-{}-{}-{}.shader_test'.format(self._stage, explicit,
                                                        from_type, to_type))

        self._filenames.append(filename)

        if not self._names_only:
            with open(filename, 'w') as test_file:
                test_file.write(TEMPLATES.get_template(
                    'execution.{}.shader_test.mako'.format(self._stage)).render_unicode(
                        ver=self._ver,
                        extensions=self._extensions,
                        amount=self._amount,
                        from_type=from_type,
                        to_type=to_type,
                        converted_from=converted_from,
                        uniform_from_type=uniform_from_type,
                        uniform_to_type=uniform_to_type,
                        conversions=conversions))

    def _gen_to_target(self):
        converted_from = 'from'
        explicit = 'implicit'

        if self._basic_type == 'b':
            explicit = 'explicit'
            self._gen_comp_test(self._conversion_type, self._target_full_type,
                                converted_from)
            converted_from = self._target_full_type + '(from)'
            conversion_values = BOOL_VALUES
            conversion_function = TestTuple.int_str_to_double_str
        elif self._basic_type == 'i':
            conversion_values = INT_VALUES
            conversion_function = TestTuple.int_str_to_double_str
        elif self._basic_type == 'u':
            conversion_values = UINT_VALUES
            conversion_function = TestTuple.int_str_to_double_str
        elif self._basic_type == 'f':
            conversion_values = FLOAT_INFS + FLOAT_NEG_ZERO + FLOAT_POS_ZERO + FLOAT_VALUES
            conversion_function = TestTuple.float_hex_to_double_hex

        conversions = []
        for value in conversion_values:
            to_value = conversion_function(value)
            item = {'from': value, 'to': to_value}
            conversions.append(item)

        self._gen_exec_test(self._conversion_type, self._target_full_type,
                            self._uniform_type, self._target_full_type,
                            explicit, converted_from, conversions)

    def _gen_from_target(self):
        converted_from = 'from'
        self._gen_comp_test(self._target_full_type, self._conversion_type,
                            converted_from)

        converted_from = self._conversion_type + '(from)'
        explicit = 'explicit'

        if self._basic_type == 'b':
            conversion_values = DOUBLE_INFS + DOUBLE_NORMAL_VALUES + DOUBLE_BOOL_VALUES
            conversion_function = TestTuple.double_hex_to_bool_str
        elif self._basic_type == 'i':
            conversion_values = DOUBLE_DENORMAL_VALUES + DOUBLE_NORMAL_VALUES + DOUBLE_INT_VALUES
            conversion_function = TestTuple.double_hex_to_int_str
        elif self._basic_type == 'u':
            conversion_values = DOUBLE_DENORMAL_VALUES + DOUBLE_NORMAL_VALUES + DOUBLE_UINT_VALUES
            conversion_function = TestTuple.double_hex_to_uint_str
        elif self._basic_type == 'f':
            conversion_values = DOUBLE_INFS + DOUBLE_FLOAT_INFS + DOUBLE_FLOAT_VALUES
            conversion_function = TestTuple.double_hex_to_float_hex

        conversions = []
        for value in DOUBLE_NEG_ZERO + DOUBLE_POS_ZERO + conversion_values:
            to_value = conversion_function(value)
            item = {'from': value, 'to': to_value}
            conversions.append(item)

        self._gen_exec_test(self._target_full_type, self._conversion_type,
                            self._target_full_type, self._uniform_type,
                            explicit, converted_from, conversions)


class ZeroSignTestTuple(TestTuple):
    """Derived class for conversion tests using the float32 and float64
       +/-0.0 values.
    """

    @staticmethod
    def all_tests(names_only):
        """Returns all the possible zero sign conversion test instances."""

        assert isinstance(names_only, bool)
        stages = ['vert', 'geom', 'frag']
        dimensions = ['1', '2', '3', '4']
        basic_types = ['f']
        glsl_ver = ['410', '420']

        if not names_only:
            for ver in glsl_ver:
                utils.safe_makedirs(get_dir_name(ver, 'execution'))

        for ver, stage, first_dimension, second_dimension, basic_type in itertools.product(
                glsl_ver,
                stages,
                dimensions,
                dimensions,
                basic_types):
            if not (first_dimension != '1' and second_dimension == '1'):
                yield ZeroSignTestTuple(ver, stage,
                                        first_dimension, second_dimension,
                                        basic_type, names_only)

    def __init__(self, ver, stage,
                 first_dimension, second_dimension,
                 basic_type, names_only):
        assert ver in ('410', '420')
        assert basic_type == 'f'
        assert not (first_dimension != '1' and second_dimension == '1')
        super(ZeroSignTestTuple, self).__init__(ver, stage,
                                                first_dimension, second_dimension,
                                                basic_type, 'd', names_only)

    def __gen_zero_sign_exec_test(self, from_type, to_type,
                                  uniform_from_type, uniform_to_type,
                                  explicit, converted_from, conversions):
        filename = os.path.join(
            get_dir_name(self._ver, 'execution'),
            '{}-conversion-{}-{}-{}-zero-sign.shader_test'.format(self._stage, explicit,
                                                                  from_type, to_type))

        self._filenames.append(filename)

        if not self._names_only:
            with open(filename, 'w') as test_file:
                test_file.write(TEMPLATES.get_template(
                    'execution-zero-sign.{}.shader_test.mako'.format(
                        self._stage)).render_unicode(
                            ver=self._ver,
                            extensions=self._extensions,
                            amount=self._amount,
                            from_type=from_type,
                            to_type=to_type,
                            converted_from=converted_from,
                            uniform_from_type=uniform_from_type,
                            uniform_to_type=uniform_to_type,
                            conversions=conversions))

    def _gen_to_target(self):
        if self._ver == '410':
            conversion_values = FLOAT_POS_ZERO
        elif self._ver == '420':
            conversion_values = FLOAT_NEG_ZERO

        conversions = []
        for value in conversion_values:
            to_value = TestTuple.float_hex_to_inv_double_hex(value)
            item = {'from': value, 'to': to_value}
            conversions.append(item)

        self.__gen_zero_sign_exec_test(self._conversion_type, self._target_full_type,
                                       self._uniform_type, self._target_full_type,
                                       'implicit', 'from', conversions)

    def _gen_from_target(self):
        if self._ver == '410':
            conversion_values = DOUBLE_POS_ZERO
        elif self._ver == '420':
            conversion_values = DOUBLE_NEG_ZERO

        conversions = []
        for value in conversion_values:
            to_value = TestTuple.double_hex_to_inv_float_hex(value)
            item = {'from': value, 'to': to_value}
            conversions.append(item)

        self.__gen_zero_sign_exec_test(self._target_full_type, self._conversion_type,
                                       self._target_full_type, self._uniform_type,
                                       'explicit', self._conversion_type + '(from)', conversions)


def main():
    """Main function."""

    parser = argparse.ArgumentParser(
        description="Generate shader tests that check the conversions from and "
        "to fp64")
    parser.add_argument(
        '--names-only',
        dest='names_only',
        action='store_true',
        default=False,
        help="Don't output files, just generate a list of filenames to stdout")
    args = parser.parse_args()

    np.seterr(divide='ignore')

    for test in (list(RegularTestTuple.all_tests(args.names_only)) +
                 list(ZeroSignTestTuple.all_tests(args.names_only))):
        test.generate_test_files()
        for filename in test.filenames:
            print(filename)


if __name__ == '__main__':
    main()
