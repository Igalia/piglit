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

"""Generate fp64 conversion tests."""

from __future__ import print_function , division, absolute_import
import optparse
import os
import sys
import itertools
import struct
import numpy as np

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

# pylint: disable=bad-whitespace,line-too-long,bad-continuation
DOUBLE_SPECIAL_VALUES = ['0xFFF0000000000000', # -inf
                         '0xFFEFFFFFFFFFFFFF', # Negative maximum normalized
                         '0xC170000000000000', # -16777216.0
                         '0xC014000000000000', # -5.0
                         '0xBFFF25CE60000000', # -1.9467300176620483
                         '0x8010000000000000', # Negative minimum normalized
                         #'0x800FFFFFFFFFFFFF', # Negative maximum denormalized -- Denormalized may be flushed to 0
                         #'0x8000000000000001', # Negative minimum denormalized -- Denormalized may be flushed to 0
                         '0x8000000000000000', # Negative underflow (-0.0)
                         '0x0000000000000000', # Positive underflow (+0.0)
                         #'0x0000000000000001', # Positive minimum denormalized -- Denormalized may be flushed to 0
                         #'0x000FFFFFFFFFFFFF', # Positive maximum denormalized -- Denormalized may be flushed to 0
                         '0x0010000000000000', # Positive minimum normalized
                         '0x3FFF25CE60000000', # +1.9467300176620483
                         '0x4014000000000000', # +5.0
                         '0x4170000000000000', # +16777216.0
                         '0x7FEFFFFFFFFFFFFF', # Positive maximum normalized
                         '0x7FF0000000000000'] # +inf

DOUBLE_FLOAT_VALUES   = ['0xC7EFFFFFF0000000', # Negative overflow (-inf)
                         '0xC7EFFFFFEFFFFFFF', # Negative maximum normalized
                         '0xB80FFFFFE0000000', # Negative minimum normalized
                         #'0xB80FFFFFDFFFFFFF', # Negative maximum denormalized -- Denormalized may be flushed to 0
                         #'0xB6A0000000000000', # Negative minimum denormalized -- Denormalized may be flushed to 0
                         '0xB69FFFFFFFFFFFFF', # Negative underflow
                         '0x369FFFFFFFFFFFFF', # Positive underflow
                         #'0x36A0000000000000', # Positive minimum denormalized -- Denormalized may be flushed to 0
                         #'0x380FFFFFDFFFFFFF', # Positive maximum denormalized -- Denormalized may be flushed to 0
                         '0x380FFFFFE0000000', # Positive minimum normalized
                         '0x47EFFFFFEFFFFFFF', # Positive maximum normalized
                         '0x47EFFFFFF0000000'] # Positive overflow (+inf)

DOUBLE_UINT_VALUES    = ['0x0000000000000001', # Positive minimum double denormalized
                         #'0x8000000000000001', # Minimum overflow                     -- Already checked
                         #'0x0000000000000000', # Minimum                 (+0)         -- Already checked
                         '0x000FFFFFFFFFFFFF', # Positive maximum double denormalized
                         '0x41DFFFFFFFC00000', # Signed int low frontier (+2147483647)
                         '0x41E0000000000000', # Signed int up frontier  (+2147483648)
                         '0x41EFFFFFFFE00000', # Maximum                 (+4294967295)
                         '0x41F0000000000000'] # Maximum overflow        (+4294967296)

DOUBLE_INT_VALUES     = ['0xC1E0000000200000', # Minimum overflow (-2147483649)
                         '0xC1E0000000000000', # Minimum          (-2147483648)
                         '0x800FFFFFFFFFFFFF', # Negative maximum double denormalized
                         #'0x8000000000000001', # Negative minimum double denormalized -- Already checked
                         #'0x0000000000000000', # Positive minimum double denormalized -- Already checked
                         '0x000FFFFFFFFFFFFF', # Positive maximum double denormalized
                         '0x41DFFFFFFFC00000', # Maximum          (+2147483647)
                         '0x41E0000000000000'] # Maximum overflow (+2147483648)

DOUBLE_BOOL_VALUES    = [#'0x8000000000000001', # Minimum negative True value -- Already checked
                         #'0x0000000000000000', # False                       -- Already checked
                         #'0x0000000000000001', # Minimum positive True value -- Already checked
                        ]

FLOAT_VALUES          = ['0xFF800000', # -inf
                         '0xFF7FFFFF', # Negative maximum normalized
                         '0xCB800000', # -16777216.0
                         '0xC0A00000', # -5.0
                         '0xBFF92E73', # -1.9467300176620483
                         '0x80800000', # Negative minimum normalized
                         #'0x807FFFFF', # Negative maximum denormalized -- Denormalized may be flushed to 0
                         #'0x80000001', # Negative minimum denormalized -- Denormalized may be flushed to 0
                         '0x80000000', # Negative underflow (-0.0)
                         '0x00000000', # Positive underflow (+0.0)
                         #'0x00000001', # Positive minimum denormalized -- Denormalized may be flushed to 0
                         #'0x007FFFFF', # Positive maximum denormalized -- Denormalized may be flushed to 0
                         '0x00800000', # Positive minimum normalized
                         '0x3FF92E73', # +1.9467300176620483
                         '0x40A00000', # +5.0
                         '0x4B800000', # +16777216.0
                         '0x7F7FFFFF', # Positive maximum normalized
                         '0x7F800000'] # +inf

UINT_VALUES          = ['0', # Minimum
                        '5',
                        '2147483647', # Signed int low frontier
                        '2147483648', # Signed int up frontier
                        '4294967295'] # Maximum

INT_VALUES            = ['-2147483648', # Minimum
                         '-5',
                         '-1',
                         '0',
                         '1',
                         '5',
                         '2147483647'] # Maximum

BOOL_VALUES          = ['0', # False
                        '1'] # True
# pylint: enable=bad-whitespace,line-too-long,bad-continuation


class TestTupla(object):
    """A double derived and other type derived tupla to generate the needed
       conversion tests.
    """

    @staticmethod
    def all_tests(compiler_dirname, execution_dirname, names_only):
        """Returns all the possible conversion test instances"""

        assert isinstance(compiler_dirname, str)
        assert isinstance(execution_dirname, str)
        assert isinstance(names_only, bool)
        stages = ['vert', 'geom', 'frag']
        dimensions = ['1', '2', '3', '4']
        basic_types = ['b', 'u', 'i', 'f']

        for stage, first_dimension, second_dimension, basic_type in itertools.product(
                stages,
                dimensions,
                dimensions,
                basic_types):
            if not (first_dimension != '1' and (second_dimension == '1' or basic_type != 'f')):
                yield TestTupla(compiler_dirname, execution_dirname,
                                stage, first_dimension, second_dimension,
                                basic_type, names_only)

    @staticmethod
    def float_to_hex(fvalue):
        """Returns the hexadecimal representation from a float32 value"""
        assert isinstance(fvalue, np.float32)
        return hex(struct.unpack('<I', struct.pack('<f', fvalue))[0])

    @staticmethod
    def double_to_hex(fvalue):
        """Returns the hexadecimal representation from a float64 value"""
        assert isinstance(fvalue, float)
        return hex(struct.unpack('<Q', struct.pack('<d', fvalue))[0]).rstrip("L")

    @staticmethod
    def hex_to_float(hstr):
        """Returns a float32 value from its hexadecimal representation"""
        assert isinstance(hstr, str)
        return struct.unpack('<f', struct.pack('<I', int(hstr, 16)))[0]

    @staticmethod
    def hex_to_double(hstr):
        """Returns a float64 value from its hexadecimal representation"""
        assert isinstance(hstr, str)
        return struct.unpack('<d', struct.pack('<Q', int(hstr, 16)))[0]

    @staticmethod
    def float_hex_to_double_hex(hstr):
        """Returns the float64 hexadecimal representation
           from a float32 hexadecimal representation
        """
        assert isinstance(hstr, str)
        double_value = TestTupla.hex_to_float(hstr)
        return TestTupla.double_to_hex(double_value)

    @staticmethod
    def int_str_to_double_str(istr):
        """Returns a float64 string from an int32 string"""
        assert isinstance(istr, str)
        return str(float(istr))

    @staticmethod
    def doble_hex_to_bool_str(hstr):
        """Returns a bool string from a float64 hexadecimal representation"""
        assert isinstance(hstr, str)
        bool_double = TestTupla.hex_to_double(hstr)
        return '1' if bool_double != 0.0 else '0'

    @staticmethod
    def doble_hex_to_int_str(hstr):
        """Returns an int32 string from a float64 hexadecimal representation"""
        assert isinstance(hstr, str)
        int_double = TestTupla.hex_to_double(hstr)
        if int_double > np.iinfo(np.dtype('int32')).max:
            return str(np.iinfo(np.dtype('int32')).max)
        if int_double < np.iinfo(np.dtype('int32')).min:
            return str(np.iinfo(np.dtype('int32')).min)
        return str(int(int_double))

    @staticmethod
    def doble_hex_to_uint_str(hstr):
        """Returns an uint32 string from a float64 hexadecimal representation"""
        assert isinstance(hstr, str)
        uint_double = TestTupla.hex_to_double(hstr)
        if uint_double > np.iinfo(np.dtype('uint32')).max:
            return str(np.iinfo(np.dtype('uint32')).max)
        if uint_double < np.iinfo(np.dtype('uint32')).min:
            return str(np.iinfo(np.dtype('uint32')).min)
        return str(int(uint_double))

    @staticmethod
    def doble_hex_to_float_hex(hstr):
        """Returns the float32 hexadecimal representation
           from a float64 hexadecimal representation
        """
        assert isinstance(hstr, str)
        float_double = np.float32(TestTupla.hex_to_double(hstr))
        return TestTupla.float_to_hex(float_double)

    def __init__(self, compiler_dirname, execution_dirname,
                 stage, first_dimension, second_dimension,
                 basic_type, names_only):
        assert isinstance(compiler_dirname, str)
        assert isinstance(execution_dirname, str)
        assert stage in ('vert', 'geom', 'frag')
        assert first_dimension in ('1', '2', '3', '4')
        assert second_dimension in ('1', '2', '3', '4')
        assert basic_type in ('b', 'u', 'i', 'f')
        assert isinstance(names_only, bool)
        assert not (first_dimension != '1' and (second_dimension == '1' or basic_type != 'f'))

        self.__compiler_dirname = compiler_dirname
        self.__execution_dirname = execution_dirname
        self.__stage = stage
        self.__first_dimension = first_dimension
        self.__second_dimension = second_dimension
        self.__basic_type = basic_type
        self.__names_only = names_only
        self.__double_type = ''
        self.__conversion_type = ''
        self.__uniform_type = ''
        self.__amount = int(first_dimension) * int(second_dimension)
        self.__filenames = []

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
                self.__conversion_type = 'bool'
                self.__uniform_type = 'int'
            elif basic_type == 'i':
                self.__conversion_type = 'int'
            elif basic_type == 'u':
                self.__conversion_type = 'uint'
            elif basic_type == 'f':
                self.__conversion_type = 'float'
            self.__double_type = 'double'
            if self.__uniform_type == '':
                self.__uniform_type = self.__conversion_type
        else:
            self.__conversion_type = (basic_type if basic_type != 'f' else '') + dimensional_type
            if basic_type == 'b':
                self.__uniform_type = 'i' + dimensional_type
            else:
                self.__uniform_type = self.__conversion_type
            self.__double_type = 'd' + dimensional_type

    def __generate_compiler_test(self, from_type, to_type, converted_from):
        filename = os.path.join(
            self.__compiler_dirname,
            '{0}-conversion-implicit-{1}-{2}-bad.{3}'.format(self.__stage, from_type, to_type,
                                                         self.__stage))

        self.__filenames.append(filename)

        if not self.__names_only:
            with open(filename, 'w') as test_file:
                test_file.write(TEMPLATES.get_template(
                    'compiler.{0}.mako'.format(self.__stage)).render_unicode(
                        from_type=from_type,
                        to_type=to_type,
                        converted_from=converted_from))

    def __generate_execution_test(self,
                                  from_type, to_type,
                                  uniform_from_type, uniform_to_type,
                                  explicit, converted_from,
                                  conversions):
        filename = os.path.join(
            self.__execution_dirname,
            '{0}-conversion-{1}-{2}-{3}.shader_test'.format(self.__stage, explicit,
                                                            from_type, to_type))

        self.__filenames.append(filename)

        if not self.__names_only:
            with open(filename, 'w') as test_file:
                test_file.write(TEMPLATES.get_template(
                    'execution.{0}.shader_test.mako'.format(self.__stage)).render_unicode(
                        amount=self.__amount,
                        from_type=from_type,
                        to_type=to_type,
                        converted_from=converted_from,
                        uniform_from_type=uniform_from_type,
                        uniform_to_type=uniform_to_type,
                        conversions=conversions))

    def __generate_to_double(self):
        converted_from = 'from'
        explicit = 'implicit'

        if self.__basic_type == 'b':
            explicit = 'explicit'
            self.__generate_compiler_test(self.__conversion_type, self.__double_type,
                                          converted_from)
            converted_from = self.__double_type + '(from)'
            conversion_values = BOOL_VALUES
            conversion_function = TestTupla.int_str_to_double_str
        elif self.__basic_type == 'i':
            conversion_values = INT_VALUES
            conversion_function = TestTupla.int_str_to_double_str
        elif self.__basic_type == 'u':
            conversion_values = UINT_VALUES
            conversion_function = TestTupla.int_str_to_double_str
        elif self.__basic_type == 'f':
            conversion_values = FLOAT_VALUES
            conversion_function = TestTupla.float_hex_to_double_hex
        else:
            assert False

        conversions = []
        for value in conversion_values:
            to_value = conversion_function(value)
            item = {'from': value, 'to': to_value}
            conversions.append(item)

        self.__generate_execution_test(self.__conversion_type, self.__double_type,
                                       self.__uniform_type, self.__double_type,
                                       explicit, converted_from,
                                       conversions)

    def __generate_from_double(self):
        converted_from = 'from'
        self.__generate_compiler_test(self.__double_type, self.__conversion_type,
                                      converted_from)

        converted_from = self.__conversion_type + '(from)'
        explicit = 'explicit'

        if self.__basic_type == 'b':
            conversion_values = DOUBLE_BOOL_VALUES
            conversion_function = TestTupla.doble_hex_to_bool_str
        elif self.__basic_type == 'i':
            conversion_values = DOUBLE_INT_VALUES
            conversion_function = TestTupla.doble_hex_to_int_str
        elif self.__basic_type == 'u':
            conversion_values = DOUBLE_UINT_VALUES
            conversion_function = TestTupla.doble_hex_to_uint_str
        elif self.__basic_type == 'f':
            conversion_values = DOUBLE_FLOAT_VALUES
            conversion_function = TestTupla.doble_hex_to_float_hex
        else:
            assert False

        conversions = []
        for value in DOUBLE_SPECIAL_VALUES + conversion_values:
            to_value = conversion_function(value)
            item = {'from': value, 'to': to_value}
            conversions.append(item)

        self.__generate_execution_test(self.__double_type, self.__conversion_type,
                                       self.__double_type, self.__uniform_type,
                                       explicit, converted_from,
                                       conversions)

    @property
    def filenames(self):
        """Returns the test file names this tupla will generate."""
        if self.__filenames == []:
            tmp = self.__names_only
            self.__names_only = True
            self.generate_test_files()
            self.__names_only = tmp
        return self.__filenames

    def generate_test_files(self):
        """Generate the GLSL parser tests."""
        self.__filenames = []

        self.__generate_to_double()
        self.__generate_from_double()


def main():
    """Main function."""
    parser = optparse.OptionParser(
        description="Generate shader tests that test the conversions from and "
                    "to fp64",
        usage="usage: %prog [-h] [--names-only]")
    parser.add_option(
        '--names-only',
        dest='names_only',
        action='store_true',
        help="Don't output files, just generate a list of filenames to stdout")

    (options, args) = parser.parse_args()

    if len(args) != 0:
        # User gave extra args.
        parser.print_help()
        sys.exit(1)

    compiler_dirname = os.path.join('spec', 'arb_gpu_shader_fp64', 'compiler',
                                    'conversion')
    execution_dirname = os.path.join('spec', 'arb_gpu_shader_fp64', 'execution',
                                     'conversion')

    if not options.names_only:
        utils.safe_makedirs(compiler_dirname)
        utils.safe_makedirs(execution_dirname)

    for test in TestTupla.all_tests(compiler_dirname, execution_dirname,
                                    bool(options.names_only)):
        test.generate_test_files()
        for filename in test.filenames:
            print(filename)


if __name__ == '__main__':
    main()
