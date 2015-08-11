#!/usr/bin/env python
# coding=utf-8
#
# Copyright © 2014 The Piglit Project
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

"""Test passing variables from the tessellation control shader to the
tessellation evaluation shader.

For every combination of varying type as scalar and as two element array and
per-vertex or per-patch varying create a test that that passes said variable
between the tessellation shader stages.

Copy a uniform value to the varying in the tessellation control shader and
compare the varying to the same uniform in the tessellation evaluation
shader.
If the values are equal draw the screen green, red otherwise.

Draw for tessellated quads. Each should cover one quarter of the screen.

This script outputs, to stdout, the name of each file it generates.

"""

from __future__ import print_function, absolute_import, division
import os
import sys
import random
import textwrap

from six.moves import range


class Test(object):
    def __init__(self, type_name, array, patch_in, name):
        """Creates a test.

        type_name -- varying type to test (e.g.: vec4, mat3x2, int, ...)
        array -- number of array elements to test, None for no array
        patch_in -- true for per-patch varying, false for per-vertex varying
        name -- name of the variable to test

        """
        self.var_name = name or 'var'
        self.use_block = 0 if patch_in else 1

        if self.var_name == 'gl_Position':
            self.var_type = 'vec4'
            self.var_array = None
            self.patch_in = False
        elif self.var_name == 'gl_PointSize':
            self.var_type = 'float'
            self.var_array = None
            self.patch_in = False
        elif self.var_name == 'gl_ClipDistance':
            self.var_type = 'float'
            self.var_array = 8
            self.patch_in = False
        else:
            self.var_type = type_name
            self.var_array = array
            self.patch_in = patch_in

        if self.built_in:
            self.interface_name = 'gl_PerVertex'
            self.interface_tcs_instance = 'gl_out'
            self.interface_tes_instance = 'gl_in'
        else:
            self.interface_name = 'tc2te_interface'
            self.interface_tcs_instance = '' if self.patch_in else 'tc2te'
            self.interface_tes_instance = '' if self.patch_in else 'tc2te'

        if self.var_array:
            self.var_type_full = self.var_type + '[{0}]'.format(self.var_array)
        else:
            self.var_type_full = self.var_type

        if self.patch_in:
            self.interface_prefix = 'patch '
            self.interface_postfix = ''
        else:
            self.interface_prefix = ''
            self.interface_postfix = '[]'

    @property
    def built_in(self):
        return self.var_name.startswith('gl_')

    @property
    def tcs_var_ref(self):
        return '[gl_InvocationID].' + self.var_name if self.interface_tcs_instance else self.var_name

    @property
    def tes_var_ref(self):
        return '[i].' + self.var_name if self.interface_tes_instance else self.var_name

    @property
    def tcs_reference_index(self):
        return 'gl_PrimitiveID' if self.patch_in else 'gl_PrimitiveID * vertices_in + gl_InvocationID'

    @property
    def tes_reference_index(self):
        return 'gl_PrimitiveID' if self.patch_in else 'gl_PrimitiveID * vertices_in + i'

    @property
    def reference_size(self):
        return 4 if self.patch_in else 12

    @property
    def uniform_string(self):
        """Returns string for loading uniform data by the shader_runner."""
        data = self.test_data()
        uniforms = ''
        if self.var_array:
            for i in range(self.reference_size):
                for j in range(self.var_array):
                    uniforms += 'uniform {0} reference[{1}].v[{2}] {3}\n'.format(
                        self.var_type, i, j, data[i*j])
        else:
            for i in range(self.reference_size):
                uniforms += 'uniform {0} reference[{1}].v {2}\n'.format(
                    self.var_type,
                    i,
                    data[i])

        #strip last newline
        return uniforms[:-1]

    def components(self):
        """Returns the number of scalar components of the used data type."""
        n = 1

        if self.var_type.startswith('mat'):
            if 'x' in self.var_type:
                n *= int(self.var_type[-1])
                n *= int(self.var_type[-3])
            else:
                n *= int(self.var_type[-1])
                n *= int(self.var_type[-1])
        elif 'vec' in self.var_type:
            n *= int(self.var_type[-1])

        return n

    def test_data(self):
        """Returns random but deterministic data as a list of strings.

        n strings are returned containing c random values, each.
        Where n is the number of vertices times the array length and
        c is the number of components in the tested scalar data type.
        """
        random.seed(17)

        if self.var_array:
            n = self.var_array * self.reference_size
        else:
            n = self.reference_size

        if self.var_type.startswith('i'):
            rand = lambda: random.randint(-0x80000000, 0x7fffffff)
        elif self.var_type.startswith('u'):
            rand = lambda: random.randint(0, 0xffffffff)
        else:
            rand = lambda: ((-1 + 2 * random.randint(0, 1)) *
                            random.randint(0, 2**23-1) *
                            2.0**(random.randint(-126, 127)))

        c = self.components()

        ret = []
        for _ in range(n):
            ret.append(" ".join(str(rand()) for _ in range(c)))

        return ret

    def filename(self):
        """Returns the file name (including path) for the test."""
        if self.built_in:
            name = self.var_name
        elif self.var_array:
            name = self.var_type + '_{0}'.format(self.var_array)
        else:
            name = self.var_type
        if self.patch_in:
            name = 'patch-' + name
        return os.path.join('spec',
                            'arb_tessellation_shader',
                            'execution',
                            'tes-input',
                            'tes-input-{0}.shader_test'.format(name))

    def generate(self):
        """Generates and writes the test to disc."""
        test = textwrap.dedent("""\
            # Test generated by:
            # {generator_command}
            # Test tessellation control shader inputs
            [require]
            GLSL >= 1.50
            GL_ARB_tessellation_shader

            [vertex shader]
            void main()
            {{
                    gl_Position = vec4(0);
            }}

            [tessellation control shader]
            #extension GL_ARB_tessellation_shader : require
            layout(vertices = 3) out;

            uniform struct S0 {{
                    {self.var_type_full} v;
            }} reference[12];

            #if {self.use_block}
            {self.interface_prefix}out {self.interface_name} {{
                    {self.var_type_full} {self.var_name};
            }} {self.interface_tcs_instance}{self.interface_postfix};
            #else
            {self.interface_prefix}out {self.var_type_full} {self.var_name};
            #endif

            void main()
            {{
                    const int vertices_in = 3;
                    {self.interface_tcs_instance}{self.tcs_var_ref} = reference[{self.tcs_reference_index}].v;
                    gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);
                    gl_TessLevelInner = float[2](1.0, 1.0);
            }}

            [tessellation evaluation shader]
            #extension GL_ARB_tessellation_shader : require
            layout(quads) in;

            uniform struct S0 {{
                    {self.var_type_full} v;
            }} reference[12];

            #if {self.use_block}
            {self.interface_prefix}in {self.interface_name} {{
                    {self.var_type_full} {self.var_name};
            }} {self.interface_tes_instance}{self.interface_postfix};
            #else
            {self.interface_prefix}in {self.var_type_full} {self.var_name};
            #endif

            out vec4 vert_color;

            void main()
            {{
                    const int vertices_in = 3;
                    const vec4 red = vec4(1, 0, 0, 1);
                    const vec4 green = vec4(0, 1, 0, 1);
                    vert_color = green;
                    for (int i = 0; i < vertices_in; ++i)
                            if ({self.interface_tes_instance}{self.tes_var_ref} != reference[{self.tes_reference_index}].v)
                                    vert_color = red;
                    vec2[3] position = vec2[3](
                            vec2(float(gl_PrimitiveID / 2) - 1.0, float(gl_PrimitiveID % 2) - 1.0),
                            vec2(float(gl_PrimitiveID / 2) - 0.0, float(gl_PrimitiveID % 2) - 1.0),
                            vec2(float(gl_PrimitiveID / 2) - 1.0, float(gl_PrimitiveID % 2) - 0.0)
                    );
                    gl_Position = vec4(position[0]
                                + (position[1] - position[0]) * gl_TessCoord[0]
                                + (position[2] - position[0]) * gl_TessCoord[1], 0.0, 1.0);
            }}

            [fragment shader]

            in vec4 vert_color;

            out vec4 frag_color;

            void main()
            {{
                    frag_color = vert_color;
            }}

            [test]
            {self.uniform_string}
            draw arrays GL_PATCHES 0 12
            relative probe rgb (0.25, 0.25) (0.0, 1.0, 0.0)
            relative probe rgb (0.75, 0.25) (0.0, 1.0, 0.0)
            relative probe rgb (0.25, 0.75) (0.0, 1.0, 0.0)
            relative probe rgb (0.75, 0.75) (0.0, 1.0, 0.0)
            """)

        test = test.format(self=self, generator_command=" ".join(sys.argv))

        filename = self.filename()
        dirname = os.path.dirname(filename)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        with open(filename, 'w') as f:
            f.write(test)


def all_tests():
    for type_name in ['float', 'vec2', 'vec3', 'vec4',
                      'mat2', 'mat3', 'mat4',
                      'mat2x3', 'mat2x4', 'mat3x2',
                      'mat3x4', 'mat4x2', 'mat4x3',
                      'int', 'ivec2', 'ivec3', 'ivec4',
                      'uint', 'uvec2', 'uvec3', 'uvec4']:
        for array in [None, 2]:
            for patch_in in [True, False]:
                yield Test(type_name, array, patch_in, name=None)
    for var in ['gl_Position', 'gl_PointSize', 'gl_ClipDistance']:
        yield Test(type_name=None, array=None, patch_in=False, name=var)


def main():
    for test in all_tests():
        test.generate()
        print(test.filename())


if __name__ == '__main__':
    main()
