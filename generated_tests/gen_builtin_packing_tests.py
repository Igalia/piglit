# coding=utf-8
# Copyright (c) 2014 Intel Corporation

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

"""This scripts generates tests for the GLSL packing functions, such as
packSnorm2x16.

In the test templates below, observe that the GLSL function's actual output is
compared against multiple expected outputs.  Given an input and a
pack/unpackfunction, there exist multiple valid outputs because the GLSL specs
permit variation in the implementation of the function. The actual output is
dependent on the GLSL compiler's and hardware's choice of rounding mode (for
example, to even or to nearest) and handling of subnormal (also called
denormalized) floating point numbers.

"""

from __future__ import print_function
import math
import optparse
import os
import sys
from collections import namedtuple
from math import copysign, fabs, fmod, frexp, isinf, isnan, modf

from numpy import int8, int16, uint8, uint16, uint32, float32

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

# pylint: disable=bad-whitespace,line-too-long
TEMPLATE_TABLE = {
    ("const", "p", "2x16"): TEMPLATES.get_template('const_pack.shader_test.mako'),
    ("const", "p",  "4x8"): TEMPLATES.get_template('const_pack.shader_test.mako'),
    ("const", "u", "2x16"): TEMPLATES.get_template('const_unpack.shader_test.mako'),
    ("const", "u",  "4x8"): TEMPLATES.get_template('const_unpack.shader_test.mako'),
    ("vs",    "p", "2x16"): TEMPLATES.get_template('vs_pack.shader_test.mako'),
    ("vs",    "p",  "4x8"): TEMPLATES.get_template('vs_pack.shader_test.mako'),
    ("vs",    "u", "2x16"): TEMPLATES.get_template('vs_unpack.shader_test.mako'),
    ("vs",    "u",  "4x8"): TEMPLATES.get_template('vs_unpack.shader_test.mako'),
    ("fs",    "p", "2x16"): TEMPLATES.get_template('fs_pack.shader_test.mako'),
    ("fs",    "p",  "4x8"): TEMPLATES.get_template('fs_pack.shader_test.mako'),
    ("fs",    "u", "2x16"): TEMPLATES.get_template('fs_unpack.shader_test.mako'),
    ("fs",    "u",  "4x8"): TEMPLATES.get_template('fs_unpack.shader_test.mako'),
}
# pylint: enable=bad-whitespace,line-too-long

# TODO: all of the invalid names should be fixed (mostly one and two letter
# variable names), but there are lots of them, and they get in the way.
# TODO: Docstrings...
# pylint: disable=invalid-name,missing-docstring

class FuncOpts(object):  # pylint: disable=too-few-public-methods
    """Options that modify the evaluation of the GLSL pack/unpack functions.

    Given an input and a pack/unpack function, there exist multiple valid
    outputs because the GLSL specs permit variation in the implementation of
    the function. The actual output is dependent on the GLSL compiler's and
    hardware's choice of rounding mode (for example, to even or to nearest).

    This class attempts to capture the permitted variation in rounding
    behavior. To select a particular behavior, pass the appropriate enum to
    the constructor, as described below.

    Rounding mode
    -------------
    For some packing functions, the GLSL ES 3.00 specification's definition of
    the function's behavior involves round(), whose behavior at
    0.5 is an implementation detail. From section 8.3 of the spec:
        The fraction 0.5 will round in a direction chosen by the
        implementation, presumably the direction that is fastest.

    The constructor parameter 'round_mode' selects the rounding behavior.
    Valid values are:
        - ROUND_TO_EVEN
        - ROUND_TO_NEAREST
    """

    ROUND_TO_EVEN = 0
    ROUND_TO_NEAREST = 1

    def __init__(self, round_mode=ROUND_TO_EVEN):
        if round_mode == FuncOpts.ROUND_TO_EVEN:
            self.__round_func = round_to_even
        elif round_mode == FuncOpts.ROUND_TO_NEAREST:
            self.__round_func = round_to_nearest
        else:
            raise Exception('Must round to even or nearest.\n'
                            'round function: {}'.format(round_mode))

    def round(self, x):
        """Round a float according to the requested rounding mode."""
        assert any(isinstance(x, T) for T in [float, float32])

        # Drop the floating-point precision from 64 to 32 bits before
        # rounding.  The loss of precision may shift the float's fractional
        # value to 0.5, which will affect the rounding.
        x = float32(x)
        return self.__round_func(x)


def clamp(x, min_, max_):
    if x < min_:
        return min_
    elif x > max_:
        return max_
    else:
        return x


def round_to_nearest(x):
    # Get fractional and integral parts.
    (f, i) = modf(x)

    if fabs(f) < 0.5:
        return i
    else:
        return i + copysign(1.0, x)


def round_to_even(x):
    # Get fractional and integral parts.
    (f, i) = modf(x)

    if fabs(f) < 0.5:
        return i
    elif fabs(f) == 0.5:
        return i + fmod(i, 2.0)
    else:
        return i + copysign(1.0, x)


def pack_2x16(pack_1x16_func, x, y, func_opts):
    """Evaluate a GLSL pack2x16 function.

    :param pack_1x16_func: the component-wise function of the GLSL pack2x16
        function
    :param x,y: each a float32
    :return: a uint32
    """
    assert isinstance(x, float32)
    assert isinstance(y, float32)

    ux = pack_1x16_func(x, func_opts)
    uy = pack_1x16_func(y, func_opts)

    assert isinstance(ux, uint16)
    assert isinstance(uy, uint16)

    return uint32((uy << 16) | ux)


def pack_4x8(pack_1x8_func, x, y, z, w, func_opts):
    # pylint: disable=too-many-arguments
    """Evaluate a GLSL pack4x8 function.

    :param pack_1x8_func: the component-wise function of the GLSL pack4x8
        function
    :param x,y,z,w: each a float32
    :return: a uint32
    """
    assert isinstance(x, float32)
    assert isinstance(y, float32)
    assert isinstance(z, float32)
    assert isinstance(w, float32)

    ux = pack_1x8_func(x, func_opts)
    uy = pack_1x8_func(y, func_opts)
    uz = pack_1x8_func(z, func_opts)
    uw = pack_1x8_func(w, func_opts)

    assert isinstance(ux, uint8)
    assert isinstance(uy, uint8)
    assert isinstance(uz, uint8)
    assert isinstance(uw, uint8)

    return uint32((uw << 24) | (uz << 16) | (uy << 8) | ux)


def unpack_2x16(unpack_1x16_func, u, _):
    """Evaluate a GLSL unpack2x16 function.

    :param unpack_1x16_func: the component-wise function of the GLSL
        unpack2x16 function
    :param u: a uint32
    :return: a 2-tuple of float32
    """
    assert isinstance(u, uint32)

    ux = uint16(u & 0xffff)
    uy = uint16(u >> 16)

    x = unpack_1x16_func(ux)
    y = unpack_1x16_func(uy)

    assert isinstance(x, float32)
    assert isinstance(y, float32)

    return (x, y)


def unpack_4x8(unpack_1x8_func, u, _):
    """Evaluate a GLSL unpack4x8 function.

    :param unpack_1x8_func: the component-wise function of the GLSL
        unpack4x8 function
    :param u: a uint32
    :return: a 4-tuple of float32
    """
    assert isinstance(u, uint32)

    ux = uint8(u & 0xff)
    uy = uint8((u >> 8) & 0xff)
    uz = uint8((u >> 16) & 0xff)
    uw = uint8((u >> 24) & 0xff)

    x = unpack_1x8_func(ux)
    y = unpack_1x8_func(uy)
    z = unpack_1x8_func(uz)
    w = unpack_1x8_func(uw)

    assert isinstance(x, float32)
    assert isinstance(y, float32)
    assert isinstance(z, float32)
    assert isinstance(w, float32)

    return (x, y, z, w)


def pack_snorm_1x8(f32, func_opts):
    """Component-wise function of packSnorm4x8."""
    assert isinstance(f32, float32)
    return uint8(int8(func_opts.round(clamp(f32, -1.0, +1.0) * 127.0)))


def pack_snorm_1x16(f32, func_opts):
    """Component-wise function of packSnorm2x16."""
    assert isinstance(f32, float32)
    return uint16(int16(func_opts.round(clamp(f32, -1.0, +1.0) * 32767.0)))


def unpack_snorm_1x8(u8):
    """Component-wise function of unpackSnorm4x8."""
    assert isinstance(u8, uint8)
    return float32(clamp(int8(u8) / 127.0, -1.0, +1.0))


def unpack_snorm_1x16(u16):
    """Component-wise function of unpackSnorm2x16."""
    assert isinstance(u16, uint16)
    return float32(clamp(int16(u16) / 32767.0, -1.0, +1.0))


def pack_unorm_1x8(f32, func_opts):
    """Component-wise function of packUnorm4x8."""
    assert isinstance(f32, float32)
    return uint8(func_opts.round(clamp(f32, 0.0, 1.0) * 255.0))


def pack_unorm_1x16(f32, func_opts):
    """Component-wise function of packUnorm2x16."""
    assert isinstance(f32, float32)
    return uint16(func_opts.round(clamp(f32, 0.0, 1.0) * 65535.0))


def unpack_unorm_1x8(u8):
    """Component-wise function of unpackUnorm4x8."""
    assert isinstance(u8, uint8)
    return float32(u8 / 255.0)


def unpack_unorm_1x16(u16):
    """Component-wise function of unpackUnorm2x16."""
    assert isinstance(u16, uint16)
    return float32(u16 / 65535.0)


def pack_half_1x16(f32, func_opts):
    """Component-wise function of packHalf2x16."""
    assert isinstance(f32, float32)

    # The bit layout of a float16 is:
    #
    #   sign:     15
    #   exponent: 10:14
    #   mantissa: 0:9
    #
    # The sign, exponent, and mantissa determine its value by:
    #
    # if e = 0 and m = 0, then zero:       (-1)^s * 0
    # if e = 0 and m != 0, then subnormal: (-1)^s * 2^(e - 14) * m / 2^10
    # if 0 < e < 31, then normal:          (-1)^s * 2^(e - 15) * (1 + m / 2^10)
    # if e = 31 and m = 0, then inf:       (-1)^s * inf
    # if e = 31 and m != 0, then nan
    #
    # where 0 <= m < 2^10.
    #
    # Some key boundary values of float16 are:
    #
    #   min_normal16  = 2^(1 - 15) * (1 + 0 / 2^10)
    #   max_normal16  = 2^(30 - 15) * (1 + 1023 / 2^10)
    #
    # The maximum float16 step value is:
    #
    #   max_step16 = 2^5
    #
    # Observe that each of the above boundary values lies in the range of
    # normal float32 values. If we represent each of the above boundary values
    # in the form returned by frexpf() for normal float32 values, 2^E
    # * F where 0.5 <= F < 1, then:
    #
    #   min_normal16 = 2^(-13) * 0.5
    #   max_normal16 = 2^16 * 0.99951171875

    # The resultant float16's sign, exponent, and mantissa bits.
    s = 0
    e = 0
    m = 0

    # Calculate sign bit.
    # Use copysign() to handle the case where x is -0.0.
    if copysign(1.0, f32) < 0.0:
        s = 1

    # To reduce the number of cases in the if-tree below, decompose `abs(f32)`
    # rather than `f32`.
    (F, E) = frexp(fabs(f32))

    # The output of frexp falls into three classes:
    #   - If f32 is NaN, then F is NaN .
    #   - If f32 is ±inf, then F is ±inf .
    #   - If f32 is ±0.0, then F is ±0.0 .
    #   - Otherwise, f32 = 2^E * F where 0.5 <= F < 1.0 .
    #
    # Since we decomposed `abs(f32)`, we only need be concerned with the
    # postive cases.
    if isnan(F):
        # The resultant float16 is NaN.
        e = 31
        m = 1
    elif isinf(F):
        # The resultant float16 is infinite.
        e = 31
        m = 0
    elif F == 0:
        # f32 is zero, therefore the resultant float16 is zero.
        e = 0
        m = 0
    elif E < -13:
        # f32 lies in the range (0.0, min_normal16). Round f32 to a nearby
        # float16 value. The resultant float16 will be either zero, subnormal,
        # or normal.
        e = 0
        m = int(func_opts.round(2**(E + 24) * F))
    elif E <= 16:
        # f32 lies in the range [min_normal16, max_normal16 + max_step16).
        # Round f32 to a nearby float16 value. The resultant float16 will be
        # either normal or infinite.
        e = int(E + 14)
        m = int(func_opts.round(2**11 * F - 2**10))
    else:
        # f32 lies in the range [max_normal16 + max_step16, inf), which is
        # outside the range of finite float16 values. The resultant float16 is
        # infinite.
        e = 31
        m = 0

    if m == 1024:
        # f32 was rounded upwards into the range of the next exponent.  This
        # correctly handles the case where f32 should be rounded up to float16
        # infinity.
        e += 1
        m = 0

    assert s == 0 or s == 1
    assert 0 <= e and e <= 31
    assert 0 <= m and m <= 1023

    return uint16((s << 15) | (e << 10) | m)


def unpack_half_1x16(u16):
    """Component-wise function of unpackHalf2x16."""
    assert isinstance(u16, uint16)

    # The bit layout of a float16 is:
    #
    #   sign:     15
    #   exponent: 10:14
    #   mantissa: 0:9
    #
    # The sign, exponent, and mantissa determine its value by:
    #
    # if e = 0 and m = 0, then zero:       (-1)^s * 0
    # if e = 0 and m != 0, then subnormal: (-1)^s * 2^(e - 14) * m / 2^10
    # if 0 < e < 31, then normal:          (-1)^s * 2^(e - 15) * (1 + m / 2^10)
    # if e = 31 and m = 0, then inf:       (-1)^s * inf
    # if e = 31 and m != 0, then nan
    #
    # where 0 <= m < 2^10.

    s = (u16 >> 15) & 0x1
    e = (u16 >> 10) & 0x1f
    m = u16 & 0x3ff

    if s == 0:
        sign = 1.0
    else:
        sign = -1.0

    if e == 0:
        return float32(sign * 2.0**(-14) * (m / 2.0**10))
    elif 1 <= e and e <= 30:
        return float32(sign * 2.0**(e - 15.0) * (1.0 + m / 2.0**10))
    elif e == 31 and m == 0:
        return float32(sign * float32("inf"))
    elif e == 31 and m != 0:
        return float32("NaN")
    else:
        raise Exception('invalid inputs')

# ----------------------------------------------------------------------------
# Inputs for GLSL functions
# ----------------------------------------------------------------------------

# This table maps GLSL pack/unpack function names to a sequence of inputs to
# the respective component-wise function. It contains four types of mappings:
#    - name of a pack2x16 function to a sequence of float32
#    - name of a pack4x8 function to a sequence of float32
#    - name of a unpack2x16 function to a sequence of uint16
#    - name of a unpack4x8 function to a sequence of uint8
full_input_table = dict()

# This table maps each GLSL pack/unpack function name to a subset of
# ``full_input_table[name]``.
#
# To sufficiently test some functions, we must test a fairly large set of
# component-wise inputs, so large that its cartesian product explodes. The
# test such functions, we test over the cartesian product of full_input_table
# and reduced_input_table. See make_inouts_for_pack_2x16.
#
reduced_input_table = dict()


def make_inputs_for_pack_snorm_2x16():
    # The domain of packSnorm2x16 is [-inf, +inf]^2. The function clamps
    # its input into the range [-1, +1]^2.
    pos = (
        0.0,  # zero
        0.1,  # near zero
        0.9,  # slightly below the clamp boundary
        1.0,  # the clamp boundary
        1.1,  # slightly above the clamp boundary
        float("+inf")
    )
    neg = tuple(reversed(tuple(-x for x in pos)))
    return tuple(float32(x) for x in pos + neg)

full_input_table["packSnorm2x16"] = make_inputs_for_pack_snorm_2x16()
reduced_input_table["packSnorm2x16"] = None

full_input_table["packSnorm4x8"] = full_input_table["packSnorm2x16"]

# XXX: Perhaps there is a better choice of test inputs?
full_input_table["unpackSnorm2x16"] = tuple(uint16(u) for u in (
    0, 1, 2, 3,
    2**15 - 1,
    2**15,
    2**15 + 1,
    2**16 - 1  # max uint16
))

# XXX: Perhaps there is a better choice of test inputs?
full_input_table["unpackSnorm4x8"] = tuple(uint8(u) for u in (
    0, 1, 2, 3,
    2**7 - 1,
    2**7,
    2**7 + 1,
    2**8 - 1  # max uint8
))

full_input_table["packUnorm2x16"] = tuple(float32(x) for x in (
    # The domain of packUnorm2x16 is [-inf, +inf]^2. The function clamps its
    # input into the range [0, 1]^2.

    "-inf",
    -0.1,  # slightly below the inner clamp boundary
    -0.0,  # infintesimally below the inner clamp boundary
    +0.0,  # the inner clamp boundary
    +0.1,  # slightly above the inner clamp boundary
    +0.9,  # slightly below the outer clamp boundary
    +1.0,  # the outer clamp boundary
    +1.1,  # slightly above the outer clamp boundary
    "+inf"
))

reduced_input_table["packUnorm2x16"] = None

full_input_table["packUnorm4x8"] = full_input_table["packUnorm2x16"]

# XXX: Perhaps there is a better choice of test inputs?
full_input_table["unpackUnorm2x16"] = full_input_table["unpackSnorm2x16"]
full_input_table["unpackUnorm4x8"] = full_input_table["unpackSnorm4x8"]


def make_inputs_for_pack_half_2x16():
    # The domain of packHalf2x16 is ([-inf, +inf] + {NaN})^2. The function
    # does not clamp its input.
    #
    # We test both -0.0 and +0.0 in order to stress the implementation's
    # handling of zero.

    subnormal_min = 2.0**(-14) * (1.0 / 2.0**10)
    normal_min = 2.0**(-14) * (1.0 + 0.0 / 2.0**10)
    normal_max = 2.0**15 * (1.0 + 1023.0 / 2.0**10)
    min_step = 2.0**(-24)
    max_step = 2.0**5

    pos = tuple(float32(x) for x in (
        # Inputs that result in 0.0 .
        0.0,
        0.0 + 0.25 * min_step,

        # A thorny input...
        #
        # if round_to_even:
        #   f16 := 0.0
        # elif round_to_nearest:
        #    f16 := subnormal_min
        #
        0.0 + 0.50 * min_step,

        # Inputs that result in a subnormal
        # float16.
        #
        0.0 + 0.75 * min_step,
        subnormal_min + 0.00 * min_step,
        subnormal_min + 0.25 * min_step,
        subnormal_min + 0.50 * min_step,
        subnormal_min + 0.75 * min_step,
        subnormal_min + 1.00 * min_step,
        subnormal_min + 1.25 * min_step,
        subnormal_min + 1.50 * min_step,
        subnormal_min + 1.75 * min_step,
        subnormal_min + 2.00 * min_step,

        normal_min - 2.00 * min_step,
        normal_min - 1.75 * min_step,
        normal_min - 1.50 * min_step,
        normal_min - 1.25 * min_step,
        normal_min - 1.00 * min_step,
        normal_min - 0.75 * min_step,

        # Inputs that result in a normal float16.
        #
        normal_min - 0.50 * min_step,
        normal_min - 0.25 * min_step,
        normal_min + 0.00 * min_step,
        normal_min + 0.25 * min_step,
        normal_min + 0.50 * min_step,
        normal_min + 0.75 * min_step,
        normal_min + 1.00 * min_step,
        normal_min + 1.25 * min_step,
        normal_min + 1.50 * min_step,
        normal_min + 1.75 * min_step,
        normal_min + 2.00 * min_step,

        2.0 * normal_min + 0.50 * min_step,
        2.0 * normal_min + 0.75 * min_step,
        2.0 * normal_min + 1.00 * min_step,

        0.5,
        1.0,
        1.5,

        normal_max - 2.00 * max_step,
        normal_max - 1.75 * max_step,
        normal_max - 1.50 * max_step,
        normal_max - 1.25 * max_step,
        normal_max - 1.00 * max_step,
        normal_max - 0.75 * max_step,
        normal_max - 0.50 * max_step,
        normal_max - 0.25 * max_step,
        normal_max + 0.00 * max_step,
        normal_max + 0.25 * max_step,

        # Inputs that result in infinity.
        #
        normal_max + 0.50 * max_step,
        normal_max + 0.75 * max_step,
        normal_max + 1.00 * max_step,
        normal_max + 2.00 * max_step,

        "+inf"))

    neg = tuple(reversed([-x for x in pos]))
    return neg + pos

full_input_table["packHalf2x16"] = make_inputs_for_pack_half_2x16()

reduced_input_table["packHalf2x16"] = tuple(float32(x) for x in (
    "-inf",
    -2.0,
    -1.0,
    -0.0,
    +0.0,
    +1.0,
    +2.0,
    "+inf"
))


def make_inputs_for_unpack_half_2x16():
    # For each of the two classes of float16 values, subnormal and normalized,
    # below are listed the exponent and mantissa of the class's boundary
    # values and some values slightly inside the bounds.
    # pylint: disable=bad-whitespace
    bounds = (
        (0,     0),  # zero
        (0,     1),  # subnormal_min
        (0,     2),  # subnormal_min + min_step
        (0,  1022),  # subnormal_max - min_step
        (0,  1023),  # subnormal_max
        (1,     0),  # normal_min
        (1,     1),  # normal_min + min_step
        (30, 1022),  # normal_max - max_step
        (30, 1023),  # normal_max
        (31,    0)   # inf
    )
    # pylint: enable=bad-whitespace

    def make_uint16(s, e, m):
        return uint16((s << 15) | (e << 10) | m)

    pos = tuple(make_uint16(0, e, m) for (e, m) in bounds)
    neg = tuple(make_uint16(1, e, m) for (e, m) in reversed(bounds))
    return neg + pos

full_input_table["unpackHalf2x16"] = make_inputs_for_unpack_half_2x16()

# ----------------------------------------------------------------------------
# Expected outputs for GLSL functions
# ----------------------------------------------------------------------------

# For a given input to a GLSL function, InOutTuple lists all valid outputs.
#
# There are multiple types of InOutTuple, described below. In each
# description, the numerical types actually refer to strings that represent
# a GLSL literal of that type.
#
#   - That for a pack2x16 function: the input is a 2-tuple of float32 and each
#     output is a uint32. For example, ``InOutTuple(input=("0.0", "0.0"),
#     valid_outputs=("0u", "0u", "0u"))``.
#
#   - That for a unpack2x16 function: the input is a uint32 and each output is
#     a 2-tuple of float32. For example, ``InOutTuple(input="0x80000000u",
#     valid_outputs=(("0.0", "-0.0"),))``.
#
InOutTuple = namedtuple("InOutTuple", ("input", "valid_outputs"))


def glsl_literal(x):
    """Convert the given number to a string that represents a GLSL literal.

    :param x: a uint32 or float32
    """
    if isinstance(x, uint32):
        return "{0}u".format(uint32(x))
    elif isinstance(x, float32):
        if math.isnan(x):
            # GLSL ES 3.00 and GLSL 4.10 do not require implementations to
            # support NaN, so we do not test it.
            raise Exception('NaN is not tested.')
        elif math.isinf(x):
            # GLSL ES 3.00 lacks a literal for infinity. However, ±1.0e256
            # suffices because it lies sufficientlyoutside the range of finite
            # float32 values.
            #
            #  From page 31 of the GLSL ES 3.00 spec:
            #
            #   If the value of the floating point number is too large (small)
            #   to be stored as a single precision value, it is converted to
            #   positive (negative) infinity.
            #
            return repr(copysign(1.0e256, x))
        elif x == 0 and copysign(1.0, x) == -1.0:
            # Workaround for numpy-1.7.0, in which repr(float32(-0.0)) does
            # not return a float literal.
            # See https://github.com/numpy/numpy/issues/2935 .
            return "-0.0"
        else:
            return repr(x)
    else:
        raise Exception('Unsupported GLSL litteral')


def make_inouts_for_pack_2x16(pack_1x16_func,
                              all_float32_inputs,
                              reduced_inputs=None):
    """Determine valid outputs for a given GLSL pack2x16 function.

    If the reduced_float32_inputs parameter is None, then it is assumed to be
    the same as all_float32_inputs.

    The set of vec2 inputs constructed by this function is the union of
    cartesian products:
      (all_float32_inputs x reduced_inputs)
      + (reduced_inputs x all_float32_inputs)

    :param pack_1x16_func: the component-wise function of the pack2x16
        function
    :param float32_inputs: a sequence of inputs to pack_1x16_func
    :return: a sequence of InOutTuple
    """
    inout_seq = []

    func_opt_seq = (FuncOpts(FuncOpts.ROUND_TO_EVEN),
                    FuncOpts(FuncOpts.ROUND_TO_NEAREST))

    if reduced_inputs is None:
        reduced_inputs = all_float32_inputs

    def add_vec2_input(x, y):
        assert isinstance(x, float32)
        assert isinstance(y, float32)

        valid_outputs = []
        for func_opts in func_opt_seq:
            u32 = pack_2x16(pack_1x16_func, x, y, func_opts)
            assert isinstance(u32, uint32)
            valid_outputs.append(glsl_literal(u32))

        inout_seq.append(
            InOutTuple(input=(glsl_literal(x), glsl_literal(y)),
                       valid_outputs=valid_outputs))

    for y in reduced_inputs:
        for x in all_float32_inputs:
            add_vec2_input(x, y)
            add_vec2_input(y, x)

    return inout_seq


def make_inouts_for_pack_4x8(pack_1x8_func, float32_inputs):
    """Determine valid outputs for a given GLSL pack4x8 function.

    :param pack_1x8_func: the component-wise function of the pack4x8
        function
    :param float32_inputs: a sequence of inputs to pack_1x8_func
    :return: a sequence of InOutTuple
    """
    inout_seq = []

    func_opt_seq = (FuncOpts(FuncOpts.ROUND_TO_EVEN),
                    FuncOpts(FuncOpts.ROUND_TO_NEAREST))

    for y in float32_inputs:
        for x in float32_inputs:
            assert isinstance(x, float32)

            valid_outputs_0 = []
            valid_outputs_1 = []
            for func_opts in func_opt_seq:
                u32_0 = pack_4x8(pack_1x8_func, x, y, x, y, func_opts)
                u32_1 = pack_4x8(pack_1x8_func, x, x, y, y, func_opts)
                assert isinstance(u32_0, uint32)
                assert isinstance(u32_1, uint32)
                valid_outputs_0.append(glsl_literal(u32_0))
                valid_outputs_1.append(glsl_literal(u32_1))

            inout_seq.append(
                InOutTuple(input=(glsl_literal(x), glsl_literal(y),
                                  glsl_literal(x), glsl_literal(y)),
                           valid_outputs=valid_outputs_0))
            inout_seq.append(
                InOutTuple(input=(glsl_literal(x), glsl_literal(x),
                                  glsl_literal(y), glsl_literal(y)),
                           valid_outputs=valid_outputs_1))
    return inout_seq


def make_inouts_for_unpack_2x16(unpack_1x16_func, uint16_inputs):
    """Determine expected outputs of a given GLSL unpack2x16 function.

    :param unpack_1x16_func: the component-wise function of the unpack2x16
        function
    :param uint16_inputs: a sequence of inputs to unpack_1x16_func
    :return: a sequence of InOutTuple
    """
    inout_seq = []
    func_opts = FuncOpts()

    for y in uint16_inputs:
        for x in uint16_inputs:
            assert isinstance(x, uint16)
            u32 = uint32((y << 16) | x)
            vec2 = unpack_2x16(unpack_1x16_func, u32, func_opts)
            assert isinstance(vec2[0], float32)
            assert isinstance(vec2[1], float32)
            inout_seq.append(
                InOutTuple(input=glsl_literal(u32),
                           valid_outputs=[(glsl_literal(vec2[0]),
                                           glsl_literal(vec2[1]))]))

    return inout_seq


def make_inouts_for_unpack_4x8(unpack_1x8_func, uint8_inputs):
    """Determine expected outputs of a given GLSL unpack4x8 function.

    :param unpack_1x8_func: the component-wise function of the unpack4x8
        function
    :param uint8_inputs: a sequence of inputs to unpack_1x8_func
    :return: a sequence of InOutTuple
    """
    inout_seq = []

    func_opts = FuncOpts()

    for y in uint8_inputs:
        for x in uint8_inputs:
            assert isinstance(x, uint8)
            u32_0 = uint32((y << 24) | (x << 16) | (y << 8) | x)
            u32_1 = uint32((y << 24) | (y << 16) | (x << 8) | x)

            valid_outputs_0 = []
            valid_outputs_1 = []
            vec4_0 = unpack_4x8(unpack_1x8_func, u32_0, func_opts)
            vec4_1 = unpack_4x8(unpack_1x8_func, u32_1, func_opts)
            assert isinstance(vec4_0[0], float32)
            assert isinstance(vec4_0[1], float32)
            assert isinstance(vec4_0[2], float32)
            assert isinstance(vec4_0[3], float32)
            assert isinstance(vec4_1[0], float32)
            assert isinstance(vec4_1[1], float32)
            assert isinstance(vec4_1[2], float32)
            assert isinstance(vec4_1[3], float32)
            valid_outputs_0.append((glsl_literal(vec4_0[0]),
                                    glsl_literal(vec4_0[1]),
                                    glsl_literal(vec4_0[2]),
                                    glsl_literal(vec4_0[3])))
            valid_outputs_1.append((glsl_literal(vec4_1[0]),
                                    glsl_literal(vec4_1[1]),
                                    glsl_literal(vec4_1[2]),
                                    glsl_literal(vec4_1[3])))

            inout_seq.append(InOutTuple(input=glsl_literal(u32_0),
                                        valid_outputs=valid_outputs_0))
            inout_seq.append(InOutTuple(input=glsl_literal(u32_1),
                                        valid_outputs=valid_outputs_1))

    return inout_seq

# This table maps GLSL pack/unpack function names to the precision of their
# return type.
result_precision_table = {
    "packSnorm2x16": "highp",
    "packSnorm4x8": "highp",
    "packUnorm2x16": "highp",
    "packUnorm4x8": "highp",
    "packHalf2x16":  "highp",
    "unpackSnorm2x16": "highp",
    "unpackSnorm4x8": "highp",
    "unpackUnorm2x16": "highp",
    "unpackUnorm4x8": "highp",
    "unpackHalf2x16":  "mediump"
}

# This table maps GLSL pack/unpack function names to a sequence of InOutTuple.
inout_table = {
    "packSnorm2x16": make_inouts_for_pack_2x16(
        pack_snorm_1x16, full_input_table["packSnorm2x16"],
        reduced_input_table["packSnorm2x16"]),
    "packSnorm4x8": make_inouts_for_pack_4x8(
        pack_snorm_1x8, full_input_table["packSnorm4x8"]),
    "packUnorm2x16": make_inouts_for_pack_2x16(
        pack_unorm_1x16, full_input_table["packUnorm2x16"],
        reduced_input_table["packUnorm2x16"]),
    "packUnorm4x8": make_inouts_for_pack_4x8(
        pack_unorm_1x8, full_input_table["packUnorm4x8"]),
    "packHalf2x16":  make_inouts_for_pack_2x16(
        pack_half_1x16, full_input_table["packHalf2x16"],
        reduced_input_table["packHalf2x16"]),
    "unpackSnorm2x16": make_inouts_for_unpack_2x16(
        unpack_snorm_1x16, full_input_table["unpackSnorm2x16"]),
    "unpackSnorm4x8": make_inouts_for_unpack_4x8(
        unpack_snorm_1x8, full_input_table["unpackSnorm4x8"]),
    "unpackUnorm2x16": make_inouts_for_unpack_2x16(
        unpack_unorm_1x16, full_input_table["unpackUnorm2x16"]),
    "unpackUnorm4x8": make_inouts_for_unpack_4x8(
        unpack_unorm_1x8, full_input_table["unpackUnorm4x8"]),
    "unpackHalf2x16": make_inouts_for_unpack_2x16(
        unpack_half_1x16, full_input_table["unpackHalf2x16"])
}


# ----------------------------------------------------------------------------
# Generate test files
# ----------------------------------------------------------------------------


FuncInfo = namedtuple('FuncInfo', ['name', 'dimension', 'result_precision',
                                   'inout_seq', 'num_valid_outputs',
                                   'vector_type', 'requirements', 'exact'])

def func_info(name, requirements):
    """Factory function for information for a GLSL pack/unpack function.

    Properties
    ----------
    - name: Name of the GLSL function, such as "packSnorm2x16".

    - dimension: Dimension of the GLSL function, such as "2x16".

    - result_precision: Precision of the GLSL function's return type, such as
      "highp".

    - inout_seq: A sequence of InOutTuple.  The generated test file will test
      all inputs listed in the sequence.

    - num_valid_outputs: The number of valid outputs for each input of
      self.inout_seq. (We assume that each input has the  same number of valid
      outputs).

    - vector_type: The type of the GLSL function's parameter  or return value.
      E.g., vec4 for a 4x8 function and vec2 for a 2x16 function.

    - requirements: A set of API/extension requirments to be listed in the
      .shader_test's [requires] section.

    - exact: Whether the generated results must be exact (e.g., 0.0 and 1.0
      should always be converted exactly).

    """

    if name.endswith("2x16"):
        dimension = "2x16"
        vector_type = "vec2"
    elif name.endswith("4x8"):
        dimension = "4x8"
        vector_type = "vec4"
    else:
        raise Exception('Invalid pack type {}'.format(name))

    inout_seq = inout_table[name]

    return FuncInfo(name, dimension, result_precision_table[name],
                    inout_seq, len(inout_seq[0].valid_outputs), vector_type,
                    requirements, name.endswith("unpackHalf2x16"))


class ShaderTest(object):
    """A .shader_test file."""

    @staticmethod
    def all_tests():
        requirements = "GLSL >= 1.30\nGL_ARB_shading_language_packing"
        ARB_shading_language_packing_funcs = (
            func_info("packSnorm2x16", requirements),
            func_info("packSnorm4x8", requirements),
            func_info("packUnorm2x16", requirements),
            func_info("packUnorm4x8", requirements),
            func_info("packHalf2x16", requirements),
            func_info("unpackSnorm2x16", requirements),
            func_info("unpackSnorm4x8", requirements),
            func_info("unpackUnorm2x16", requirements),
            func_info("unpackUnorm4x8", requirements),
            func_info("unpackHalf2x16", requirements)
            )

        requirements = "GL ES >= 3.0\nGLSL ES >= 3.00"
        glsl_es_300_funcs = (
            func_info("packSnorm2x16", requirements),
            func_info("packUnorm2x16", requirements),
            func_info("packHalf2x16", requirements),
            func_info("unpackSnorm2x16", requirements),
            func_info("unpackUnorm2x16", requirements),
            func_info("unpackHalf2x16", requirements)
            )

        execution_stages = ("const", "vs", "fs")

        for s in execution_stages:
            for f in glsl_es_300_funcs:
                yield ShaderTest(f, s, "glsl-es-3.00")
            for f in ARB_shading_language_packing_funcs:
                yield ShaderTest(f, s, "ARB_shading_language_packing")

    def __init__(self, funcinfo, execution_stage, api):
        assert isinstance(funcinfo, FuncInfo)
        assert execution_stage in ("const", "vs", "fs")
        assert api in ("glsl-es-3.00", "ARB_shading_language_packing")

        self.__template = TEMPLATE_TABLE[(execution_stage,
                                          funcinfo.name[0],
                                          funcinfo.dimension)]
        self.__func_info = funcinfo
        self.__filename = os.path.join(
            "spec",
            api.lower(),
            "execution",
            "built-in-functions",
            "{0}-{1}.shader_test".format(execution_stage, funcinfo.name))

    @property
    def filename(self):
        return self.__filename

    def write_file(self):
        dirname = os.path.dirname(self.filename)
        utils.safe_makedirs(dirname)

        with open(self.filename, "w") as f:
            f.write(self.__template.render_unicode(func=self.__func_info))


def main():
    parser = optparse.OptionParser(
        description="Generate shader tests that test the built-inpacking "
                    "functions",
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

    for test in ShaderTest.all_tests():
        print(test.filename)

        # Some test files take a long time to generate, so provide status
        # updates to the user immediately.
        sys.stdout.flush()

        if not options.names_only:
            test.write_file()

if __name__ == '__main__':
    main()
