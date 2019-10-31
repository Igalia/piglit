# coding=utf-8
# Copyright (c) 2014 Intel Corporation
# Copyright (c) 2018 Advanced Micro Devices, Inc.

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

""" Generate spec/EXT_gpu_shader4 tests """

import os
import collections

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

vec = ['vec2', 'vec3', 'vec4']
ivec = ['ivec2', 'ivec3', 'ivec4']
uvec = ['uvec2', 'uvec3', 'uvec4']

genType = ['float'] + vec
genIType = ['int'] + ivec
genUType = ['unsigned int'] + uvec

ShiftOpTypes = [
    ('int', 'int'), ('unsigned int', 'unsigned int'),
    ('ivec2', 'ivec2'), ('ivec3', 'ivec3'), ('ivec4', 'ivec4'),
    ('uvec2', 'uvec2'), ('uvec3', 'uvec3'), ('uvec4', 'uvec4'),

    ('ivec2', 'int'), ('ivec3', 'int'), ('ivec4', 'int'),
    ('uvec2', 'unsigned int'), ('uvec3', 'unsigned int'), ('uvec4', 'unsigned int'),
]

BinaryOpTypes = ShiftOpTypes + [
    ('int', 'ivec2'), ('int', 'ivec3'), ('int', 'ivec4'),
    ('unsigned int', 'uvec2'), ('unsigned int', 'uvec3'), ('unsigned int', 'uvec4'),
]

OperatorParams = collections.namedtuple(
    'OperatorParams', ['name', 'op', 'types'])

UNARY_OP_TESTS = [
    OperatorParams('bitwise-not', '~', genIType + genUType),
]

BINARY_OP_TESTS = [
    OperatorParams('mod', '%', BinaryOpTypes),
    OperatorParams('bitwise-and', '&', BinaryOpTypes),
    OperatorParams('bitwise-or', '|', BinaryOpTypes),
    OperatorParams('xor', '^', BinaryOpTypes),
    OperatorParams('lshift', '<<', ShiftOpTypes),
    OperatorParams('rshift', '>>', ShiftOpTypes),
]


Type1Params = collections.namedtuple(
    'Type1Params', ['func', 'type1'])
Type2Params = collections.namedtuple(
    'Type2Params', ['func', 'type1', 'type2'])

UNARY_TESTS = [
    Type1Params('abs', genIType),
    Type1Params('sign', genIType),
    Type1Params('truncate', genType),
    Type1Params('round', genType),
]

BINARY_TESTS = [
    Type1Params('min', genIType),
    Type2Params('min', ivec, 'int'),
    Type1Params('min', genUType),
    Type2Params('min', uvec, 'unsigned int'),

    Type1Params('max', genIType),
    Type2Params('max', ivec, 'int'),
    Type1Params('max', genUType),
    Type2Params('max', uvec, 'unsigned int'),
]

TERNARY_TESTS = [
    Type1Params('clamp', genIType),
    Type2Params('clamp', ivec, 'int'),
    Type1Params('clamp', genUType),
    Type2Params('clamp', uvec, 'unsigned int'),
]

VEC_COMPARE_TESTS = [
    Type1Params('lessThan', uvec),
    Type1Params('lessThanEqual', uvec),
    Type1Params('greaterThan', uvec),
    Type1Params('greaterThanEqual', uvec),
    Type1Params('equal', uvec),
    Type1Params('notEqual', uvec),
]


FLOAT = ['']
INT = ['i', 'u']
ALL = FLOAT + INT

FetchParams = collections.namedtuple(
    'FetchParams', ['sampler', 'coord', 'offsetcoord', 'variants'])

FETCH_TESTS = [
    FetchParams('1D', 'int', 'int', ALL),
    FetchParams('2D', 'ivec2', 'ivec2', ALL),
    FetchParams('3D', 'ivec3', 'ivec3', ALL),
    FetchParams('2DRect', 'ivec2', 'ivec2', ALL),
    FetchParams('1DArray', 'ivec2', 'int', ALL),
    FetchParams('2DArray', 'ivec3', 'ivec2', ALL),
    FetchParams('Buffer', 'int', '', ALL),
]

SIZE_TESTS = FETCH_TESTS + [
    FetchParams('Cube', 'ivec2', '', ALL),
]


TextureParams = collections.namedtuple(
    'TextureParams', ['func', 'sampler', 'coord', 'offsetcoord', 'variants'])

# Variants: std, bias, lod
COMMON_TEXTURE_TESTS = [
    # Inherited from GLSL 1.10, test integer textures
    TextureParams('texture1D', '1D', 'float', 'int', INT),
    TextureParams('texture1DProj', '1D', 'vec2', 'int', INT),
    TextureParams('texture1DProj', '1D', 'vec4', 'int', INT),

    TextureParams('texture2D', '2D', 'vec2', 'ivec2', INT),
    TextureParams('texture2DProj', '2D', 'vec3', 'ivec2', INT),
    TextureParams('texture2DProj', '2D', 'vec4', 'ivec2', INT),

    TextureParams('texture3D', '3D', 'vec3', 'ivec3', INT),
    TextureParams('texture3DProj', '3D', 'vec4', 'ivec3', INT),

    TextureParams('textureCube', 'Cube', 'vec3', '', INT),

    # Added by GL_EXT_gpu_shader4
    TextureParams('texture1DArray', '1DArray', 'vec2', 'int', ALL),
    TextureParams('texture2DArray', '2DArray', 'vec3', 'ivec2', ALL),

    TextureParams('shadow1D', '1DShadow', 'vec3', 'int', FLOAT),
    TextureParams('shadow2D', '2DShadow', 'vec3', 'ivec2', FLOAT),
    TextureParams('shadow1DProj', '1DShadow', 'vec4', 'int', FLOAT),
    TextureParams('shadow2DProj', '2DShadow', 'vec4', 'ivec2', FLOAT),
    TextureParams('shadow1DArray', '1DArrayShadow', 'vec3', 'int', FLOAT),
]

TEXTURE_TESTS = COMMON_TEXTURE_TESTS + [
    TextureParams('shadow2DArray', '2DArrayShadow', 'vec4', 'ivec2', FLOAT),
    TextureParams('shadowCube', 'CubeShadow', 'vec4', '', FLOAT),

    # Inherited from ARB_texture_rectangle, test integer textures
    TextureParams('texture2DRect', '2DRect', 'vec2', 'ivec2', INT),
    TextureParams('texture2DRectProj', '2DRect', 'vec3', 'ivec2', INT),
    TextureParams('texture2DRectProj', '2DRect', 'vec4', 'ivec2', INT),
    TextureParams('shadow2DRect', '2DRectShadow', 'vec3', 'ivec2', FLOAT),
    TextureParams('shadow2DRectProj', '2DRectShadow', 'vec4', 'ivec2', FLOAT),
]

LOD_TESTS = COMMON_TEXTURE_TESTS
BIAS_TESTS = COMMON_TEXTURE_TESTS


GradParams = collections.namedtuple(
    'GradParams', ['coord', 'grad', 'offsetcoord', 'sampler', 'func', 'variants'])

GRAD_TESTS = [
    GradParams('float', 'float', 'int', '1D', 'texture1D', ALL),
    GradParams('vec2', 'float', 'int', '1D', 'texture1DProj', ALL),
    GradParams('vec4', 'float', 'int', '1D', 'texture1DProj', ALL),
    GradParams('vec2', 'float', 'int', '1DArray', 'texture1DArray', ALL),

    GradParams('vec2', 'vec2', 'ivec2', '2D', 'texture2D', ALL),
    GradParams('vec3', 'vec2', 'ivec2', '2D', 'texture2DProj', ALL),
    GradParams('vec4', 'vec2', 'ivec2', '2D', 'texture2DProj', ALL),
    GradParams('vec3', 'vec2', 'ivec2', '2DArray', 'texture2DArray', ALL),

    GradParams('vec3', 'vec3', 'ivec3', '3D', 'texture3D', ALL),
    GradParams('vec4', 'vec3', 'ivec3', '3D', 'texture3DProj', ALL),

    GradParams('vec3', 'vec3', '', 'Cube', 'textureCube', ALL),

    GradParams('vec3', 'float', 'int', '1DShadow', 'shadow1D', FLOAT),
    GradParams('vec4', 'float', 'int', '1DShadow', 'shadow1DProj', FLOAT),
    GradParams('vec3', 'float', 'int', '1DArrayShadow', 'shadow1DArray', FLOAT),

    GradParams('vec3', 'vec2', 'ivec2', '2DShadow', 'shadow2D', FLOAT),
    GradParams('vec4', 'vec2', 'ivec2', '2DShadow', 'shadow2DProj', FLOAT),
    GradParams('vec4', 'vec2', 'ivec2', '2DArrayShadow', 'shadow2DArray', FLOAT),

    GradParams('vec2', 'vec2', 'ivec2', '2DRect', 'texture2DRect', ALL),
    GradParams('vec3', 'vec2', 'ivec2', '2DRect', 'texture2DRectProj', ALL),
    GradParams('vec4', 'vec2', 'ivec2', '2DRect', 'texture2DRectProj', ALL),

    GradParams('vec3', 'vec2', 'ivec2', '2DRectShadow', 'shadow2DRect', FLOAT),
    GradParams('vec4', 'vec2', 'ivec2', '2DRectShadow', 'shadow2DRectProj', FLOAT),

    GradParams('vec4', 'vec3', '', 'CubeShadow', 'shadowCube', FLOAT),
]

def get_swizzle(vtype):
    """
    Expand the type to vec4.
    """
    if '3' in vtype:
        return '.xyzz';
    if '2' in vtype:
        return '.xyyy';
    return '';

def get_bvec(vtype):
    """
    Return the corresponding bvec type.
    """
    if '4' in vtype:
        return 'bvec4';
    if '3' in vtype:
        return 'bvec3';
    if '2' in vtype:
        return 'bvec2';
    return 'invalid';

def get_extensions(sampler):
    """ If this test uses GL_ARB_texture_rectangle add it

    GL_ARB_texture_rectangle is an odd extension, it is on by default, so don't
    generate a #extension in the shader, just in the config block.

    """
    if 'Rect' in sampler:
        return 'GL_EXT_gpu_shader4 GL_ARB_texture_rectangle'

    if 'Array' in sampler:
        return 'GL_EXT_gpu_shader4 GL_EXT_texture_array'

    if 'Buffer' in sampler:
        return 'GL_EXT_gpu_shader4 GL_EXT_texture_buffer_object'

    return 'GL_EXT_gpu_shader4'


def main():
    """ Main function

    Writes tests to generated_tests/spec/ext_gpu_shader4/ directory

    """
    dirname = 'spec/ext_gpu_shader4/compiler'
    utils.safe_makedirs(dirname)

    for params in UNARY_OP_TESTS:
        for type1 in params.types:
            name = os.path.join(
                dirname,
                "{func}-{type1}".format(
                    func=params.name,
                    type1=type1.replace(' ', '_')))

            for stage in ['frag', 'vert']:
                filename = '{0}.{1}'.format(name, stage)
                print(filename)
                with open(filename, 'w+') as f:
                    f.write(TEMPLATES.get_template(
                        '{0}.{1}.mako'.format('unary_op', stage)).render_unicode(
                            param=params,
                            type1=type1,
                            swizzle=get_swizzle(type1)))

    for params in BINARY_OP_TESTS:
        for types in params.types:
            name = os.path.join(
                dirname,
                "{func}-{type1}-{type2}".format(
                    func=params.name,
                    type1=types[0].replace(' ', '_'),
                    type2=types[1].replace(' ', '_')))
            result_type = types[0] if 'int' in types[1] else types[1]

            for stage in ['frag', 'vert']:
                filename = '{0}.{1}'.format(name, stage)
                print(filename)
                with open(filename, 'w+') as f:
                    f.write(TEMPLATES.get_template(
                        '{0}.{1}.mako'.format('binary_op', stage)).render_unicode(
                            param=params,
                            types=types,
                            result_type=result_type,
                            swizzle=get_swizzle(result_type)))

    for tests in [('unary', UNARY_TESTS),
                  ('binary', BINARY_TESTS),
                  ('ternary', TERNARY_TESTS),
                  ('vec_compare', VEC_COMPARE_TESTS)]:
        for params in tests[1]:
            for type1 in params.type1:
                name = os.path.join(
                    dirname,
                    "{func}-{type1}{type2}".format(
                        func=params.func,
                        type1=type1.replace(' ', '_'),
                        type2=('-' + params.type2 if 'type2' in params._fields else '').replace(' ', '_')))

                for stage in ['frag', 'vert']:
                    filename = '{0}.{1}'.format(name, stage)
                    print(filename)
                    with open(filename, 'w+') as f:
                        f.write(TEMPLATES.get_template(
                            '{0}.{1}.mako'.format(tests[0], stage)).render_unicode(
                                param=params,
                                type1=type1,
                                type2=params.type2 if 'type2' in params._fields else type1,
                                swizzle=get_swizzle(type1),
                                bvec=get_bvec(type1)))

    for tests in [('texture_size', SIZE_TESTS, ''),
                  ('texel_fetch', FETCH_TESTS, ''),
                  ('tex', TEXTURE_TESTS, ''),
                  ('tex_bias', BIAS_TESTS, ''),
                  ('tex_lod', LOD_TESTS, ''),
                  ('tex_grad', GRAD_TESTS, ''),
                  ('texel_fetch', FETCH_TESTS, 'Offset'),
                  ('tex', TEXTURE_TESTS, 'Offset'),
                  ('tex_bias', BIAS_TESTS, 'Offset'),
                  ('tex_lod', LOD_TESTS, 'Offset'),
                  ('tex_grad', GRAD_TESTS, 'Offset')]:
        for params in tests[1]:
            if tests[2] == 'Offset' and params.offsetcoord == '':
                continue

            for prefix in params.variants:
                name = os.path.join(
                    dirname,
                    "{0}{func}{offset}-{prefix}sampler{sampler}-{coord}".format(
                        tests[0],
                        func='-' + params.func if 'func' in params._fields else '',
                        offset=tests[2],
                        prefix=prefix,
                        sampler=params.sampler,
                        coord=params.coord))

                stages = ['frag']
                if tests[0] != 'tex_bias':
                    stages = stages + ['vert']

                for stage in stages:
                    filename = '{0}.{1}'.format(name, stage)
                    print(filename)
                    with open(filename, 'w+') as f:
                        f.write(TEMPLATES.get_template(
                            '{0}.{1}.mako'.format(tests[0], stage)).render_unicode(
                                param=params,
                                extensions=get_extensions(params.sampler),
                                prefix=prefix,
                                swizzle=get_swizzle(params.coord),
                                offset=tests[2]))

if __name__ == '__main__':
    main()
