# coding=utf-8
#
# Copyright © 2013 Intel Corporation
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

import struct
import os
import os.path
from mako.template import Template
from textwrap import dedent

def floatBitsToInt(f):
    return struct.unpack('i', struct.pack('f', f))[0]

def floatBitsToUint(f):
    return struct.unpack('I', struct.pack('f', f))[0]

def intBitsToFloat(i):
    return struct.unpack('f', struct.pack('i', i))[0]

def uintBitsToFloat(u):
    return struct.unpack('f', struct.pack('I', u))[0]

def passthrough(f):
    return f

def neg(num):
    return -num

def neg_abs(num):
    return -abs(num)

def vec4(f):
    return [f, f, f, f]

test_data = {
    # Interesting floating-point inputs
    'mixed':                        (2.0, 9.5, -4.5, -25.0),
    '0.0':                          vec4( 0.0), # int 0
    '-0.0':                         vec4(-0.0), # INT_MIN
    '1.0':                          vec4( 1.0),
    '-1.0':                         vec4(-1.0),
    'normalized smallest':          vec4( 1.1754944e-38),
    'normalized smallest negative': vec4(-1.1754944e-38),
    'normalized largest':           vec4( 3.4028235e+38),
    'normalized largest negative':  vec4(-3.4028235e+38)

    # Don't test +inf or -inf, since we don't have a way to pass them via
    # shader_runner [test] sections. Don't test NaN, since it has many
    # representations. Don't test subnormal values, since hardware might
    # flush them to zero.
}

# in_func: Function to convert floating-point data in test_data (above) into
#          input (given) data to pass the shader.
# out_func: Function to convert floating-point data in test_data (above) into
#           output (expected) data to pass the shader.

funcs = {
    'floatBitsToInt': {
        'in_func':  passthrough,
        'out_func': floatBitsToInt,
        'input':    'vec4',
        'output':   'ivec4'
    },
    'floatBitsToUint': {
        'in_func':  passthrough,
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

modifier_funcs = {
    '':            passthrough,
    'abs':         abs,
    'neg':         neg,
    'neg_abs':     neg_abs
}

requirements = {
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

template = Template(dedent("""\
    [require]
    GLSL >= ${version}
    % for extension in extensions:
    ${extension}
    % endfor

    [vertex shader]
    % if execution_stage == 'vs':
    % for extension in extensions:
    #extension ${extension}: enable
    % endfor

    uniform ${input_type} given;
    uniform ${output_type} expected;
    out vec4 color;
    % endif

    in vec4 vertex;

    void main() {
        gl_Position = vertex;

        % if execution_stage == 'vs':
        color = vec4(0.0, 1.0, 0.0, 1.0);

        if (expected.x != ${func}(${in_modifier_func}(given.x)))
                color.r = 1.0;
        if (expected.xy != ${func}(${in_modifier_func}(given.xy)))
                color.r = 1.0;
        if (expected.xyz != ${func}(${in_modifier_func}(given.xyz)))
                color.r = 1.0;
        if (expected != ${func}(${in_modifier_func}(given)))
                color.r = 1.0;
        % endif
    }

    [fragment shader]
    % if execution_stage == 'fs':
    % for extension in extensions:
    #extension ${extension}: enable
    % endfor

    uniform ${input_type} given;
    uniform ${output_type} expected;
    % else:
    in vec4 color;
    % endif

    out vec4 frag_color;

    void main() {
        % if execution_stage == 'fs':
        frag_color = vec4(0.0, 1.0, 0.0, 1.0);

        if (expected.x != ${func}(${in_modifier_func}(given.x)))
                frag_color.r = 1.0;
        if (expected.xy != ${func}(${in_modifier_func}(given.xy)))
                frag_color.r = 1.0;
        if (expected.xyz != ${func}(${in_modifier_func}(given.xyz)))
                frag_color.r = 1.0;
        if (expected != ${func}(${in_modifier_func}(given)))
                frag_color.r = 1.0;
        % else:
        frag_color = color;
        % endif
    }

    [vertex data]
    vertex/float/2
    -1.0 -1.0
     1.0 -1.0
     1.0  1.0
    -1.0  1.0

    [test]
    % for name, data in sorted(test_data.iteritems()):
    % if name == '-0.0' and in_modifier_func != '' and func == 'intBitsToFloat':
    # ${in_modifier_func}(INT_MIN) doesn't fit in a 32-bit int. Cannot test.
    % else:
    # ${name}
    uniform ${input_type} given ${' '.join(str(in_func(d)) for d in data)}
    uniform ${output_type} expected ${' '.join(str(out_func(modifier_func(in_func(d)))) for d in data)}
    draw arrays GL_TRIANGLE_FAN 0 4
    probe all rgba 0.0 1.0 0.0 1.0
    % endif

    % endfor
"""))

for api, requirement in requirements.iteritems():
    version = requirement['version']
    extensions = [requirement['extension']] if requirement['extension'] else []

    for func, attrib in funcs.iteritems():
        in_func = attrib['in_func']
        out_func = attrib['out_func']
        input_type = attrib['input']
        output_type = attrib['output']

        for execution_stage in ('vs', 'fs'):
            file_extension = 'frag' if execution_stage == 'fs' else 'vert'

            for in_modifier_func, modifier_func in modifier_funcs.iteritems():
                # Modifying the sign of an unsigned number doesn't make sense.
                if func == 'uintBitsToFloat' and in_modifier_func != '':
                    continue

                modifier_name = '-' + in_modifier_func if in_modifier_func != '' else ''
                filename = os.path.join('spec',
                                        api.lower(),
                                        'execution',
                                        'built-in-functions',
                                        "{0}-{1}{2}.shader_test".format(execution_stage,
                                                                        func,
                                                                        modifier_name))
                print filename

                dirname = os.path.dirname(filename)
                if not os.path.exists(dirname):
                    os.makedirs(dirname)

                if in_modifier_func == 'neg':
                    in_modifier_func = '-'
                elif in_modifier_func == 'neg_abs':
                    in_modifier_func = '-abs'

                f = open(filename, 'w')
                f.write(template.render(version=version,
                                        extensions=extensions,
                                        execution_stage=execution_stage,
                                        func=func,
                                        modifier_func=modifier_func,
                                        in_modifier_func=in_modifier_func,
                                        in_func=in_func,
                                        out_func=out_func,
                                        input_type=input_type,
                                        output_type=output_type,
                                        test_data=test_data))
                f.close()
