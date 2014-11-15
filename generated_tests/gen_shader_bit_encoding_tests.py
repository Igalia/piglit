# coding=utf-8
#
# Copyright © 2013, 2014 Intel Corporation
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

from __future__ import print_function
import struct
import os
from operator import neg

from templates import template_file

TEMPLATE = template_file(os.path.basename(os.path.splitext(__file__)[0]),
                         'template.shader_test.mako')


def floatBitsToInt(f):  # pylint: disable=invalid-name
    return struct.unpack('i', struct.pack('f', f))[0]


def floatBitsToUint(f):  # pylint: disable=invalid-name
    return struct.unpack('I', struct.pack('f', f))[0]


def intBitsToFloat(i):  # pylint: disable=invalid-name
    return struct.unpack('f', struct.pack('i', i))[0]


def uintBitsToFloat(u):  # pylint: disable=invalid-name
    return struct.unpack('f', struct.pack('I', u))[0]


def neg_abs(num):
    return neg(abs(num))


def vec4(f):
    return [f, f, f, f]

# pylint: disable=bad-whitespace
TEST_DATA = {
    # Interesting floating-point inputs
    'mixed':                        (2.0, 9.5, -4.5, -25.0),
    '0.0':                          vec4( 0.0),  # int 0
    '-0.0':                         vec4(-0.0),  # INT_MIN
    '1.0':                          vec4( 1.0),
    '-1.0':                         vec4(-1.0),
    'normalized smallest':          vec4( 1.1754944e-38),
    'normalized smallest negative': vec4(-1.1754944e-38),
    'normalized largest':           vec4( 3.4028235e+38),
    'normalized largest negative':  vec4(-3.4028235e+38),

    # Don't test +inf or -inf, since we don't have a way to pass them via
    # shader_runner [test] sections. Don't test NaN, since it has many
    # representations. Don't test subnormal values, since hardware might
    # flush them to zero.
}
# pylint: enable=bad-whitespace

# in_func: Function to convert floating-point data in test_data (above) into
#          input (given) data to pass the shader.
# out_func: Function to convert floating-point data in test_data (above) into
#           output (expected) data to pass the shader.

FUNCS = {
    'floatBitsToInt': {
        'in_func':  lambda x: x,
        'out_func': floatBitsToInt,
        'input':    'vec4',
        'output':   'ivec4'
    },
    'floatBitsToUint': {
        'in_func':  lambda x: x,
        'out_func': floatBitsToUint,
        'input':    'vec4',
        'output':   'uvec4'
    },
    'intBitsToFloat': {
        'in_func':  floatBitsToInt,
        'out_func': intBitsToFloat,
        'input':    'ivec4',
        'output':   'vec4'
    },
    'uintBitsToFloat': {
        'in_func':  floatBitsToUint,
        'out_func': uintBitsToFloat,
        'input':    'uvec4',
        'output':   'vec4'
    }
}

MODIFIER_FUNCS = {
    '':            lambda x: x,
    'abs':         abs,
    'neg':         neg,
    'neg_abs':     neg_abs,
}

REQUIREMENTS = {
    'ARB_shader_bit_encoding': {
        'version': '1.30',
        'extension': 'GL_ARB_shader_bit_encoding'
    },
    'ARB_gpu_shader5': {
        'version': '1.50',
        'extension': 'GL_ARB_gpu_shader5'
    },
    'glsl-3.30': {
        'version': '3.30',
        'extension': ''
    }
}


def main():
    """main function."""
    # pylint: disable=line-too-long
    for api, requirement in REQUIREMENTS.iteritems():
        version = requirement['version']
        extensions = [requirement['extension']] if requirement['extension'] else []

        dirname = os.path.join('spec', api.lower(), 'execution',
                               'built-in-functions')
        if not os.path.exists(dirname):
            os.makedirs(dirname)

        for func, attrib in FUNCS.iteritems():
            for execution_stage in ('vs', 'fs'):
                for in_modifier_func, modifier_func in MODIFIER_FUNCS.iteritems():
                    # Modifying the sign of an unsigned number doesn't make sense.
                    if func == 'uintBitsToFloat' and in_modifier_func != '':
                        continue

                    modifier_name = '-' + in_modifier_func if in_modifier_func != '' else ''
                    filename = os.path.join(
                        dirname,
                        "{0}-{1}{2}.shader_test".format(execution_stage, func,
                                                        modifier_name))
                    print(filename)

                    if in_modifier_func == 'neg':
                        in_modifier_func = '-'
                    elif in_modifier_func == 'neg_abs':
                        in_modifier_func = '-abs'

                    with open(filename, 'w') as f:
                        f.write(TEMPLATE.render(
                            version=version,
                            extensions=extensions,
                            execution_stage=execution_stage,
                            func=func,
                            modifier_func=modifier_func,
                            in_modifier_func=in_modifier_func,
                            in_func=attrib['in_func'],
                            out_func=attrib['out_func'],
                            input_type=attrib['input'],
                            output_type=attrib['output'],
                            test_data=TEST_DATA))


if __name__ == '__main__':
    main()
