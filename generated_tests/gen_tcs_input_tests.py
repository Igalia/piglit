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

"""Test passing variables from the vertex shader to the tessellation control
shader.

For every varying type create one tests that passes a scalar of that type
and one test that passes a two element array.
Copy a uniform value to the varying in the vertex shader and compare the
varying to the same uniform in the tessellation control shader.
If the values are equal draw the screen green, red otherwise.

Draw for tessellated quads. Each should cover one quarter of the screen.

This script outputs, to stdout, the name of each file it generates.

"""

import os, sys, random


class Test(object):
    def __init__(self, type_name, array, name):
        """Creates a test.

        type_name -- varying type to test (e.g.: vec4, mat3x2, int, ...)
        array -- number of array elements to test, None for no array
        name -- name of the variable to test

        """
        self.var_name = name or 'var'

        if self.var_name == 'gl_Position':
            self.var_type = 'vec4'
            self.var_array = None
        elif self.var_name == 'gl_PointSize':
            self.var_type = 'float'
            self.var_array = None
        elif self.var_name == 'gl_ClipDistance':
            self.var_type = 'float'
            self.var_array = 8
        else:
            self.var_type = type_name
            self.var_array = array

        if self.built_in:
            self.interface_name = 'gl_PerVertex'
            self.interface_vs_instance = ''
            self.interface_tcs_instance = 'gl_in'
        else:
            self.interface_name = 'v2tc_interface'
            self.interface_vs_instance = ''
            self.interface_tcs_instance = 'v2tc'

        if self.var_array:
            self.var_type_full = self.var_type + '[{0}]'.format(self.var_array)
        else:
            self.var_type_full = self.var_type

    @property
    def built_in(self):
        return self.var_name.startswith('gl_')

    @property
    def vs_var_ref(self):
        return '.' + self.var_name if self.interface_vs_instance else self.var_name

    @property
    def tcs_var_ref(self):
        return '.' + self.var_name if self.interface_tcs_instance else self.var_name

    @property
    def uniform_string(self):
        """Returns string for loading uniform data by the shader_runner."""
        data = self.test_data()
        uniforms = ''
        if self.var_array:
            for i in xrange(12):
                for j in xrange(self.var_array):
                    uniforms += 'uniform {0} reference[{1}].v[{2}] {3}\n'.format(
                            self.var_type, i, j, data[i*j])
        else:
            for i in xrange(12):
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
            n = self.var_array * 12
        else:
            n = 12

        if self.var_type.startswith('i'):
            rand = lambda : random.randint(-0x80000000, 0x7fffffff)
        elif self.var_type.startswith('u'):
            rand = lambda : random.randint(0, 0xffffffff)
        else:
            rand = lambda : ((-1 + 2 * random.randint(0, 1)) *
                             random.randint(0, 2**23-1) *
                             2.0**(random.randint(-126, 127)))

        c = self.components()

        ret = []
        for i in xrange(n):
            ret.append(" ".join(str(rand()) for _ in xrange(c)))

        return ret

    def filename(self):
        """Returns the file name (including path) for the test."""
        if self.built_in:
            name = self.var_name
        elif self.var_array:
            name = self.var_type + '_{0}'.format(self.var_array)
        else:
            name = self.var_type
        return os.path.join('spec',
                            'arb_tessellation_shader',
                            'execution',
                            'tcs-input',
                            'tcs-input-{0}.shader_test'.format(name))

    def generate(self):
        """Generates and writes the test to disc."""
        test = \
"""# Test generated by:
# {generator_command}
# Test tessellation control shader inputs
[require]
GLSL >= 1.50
GL_ARB_tessellation_shader

[vertex shader]
uniform struct S0 {{
        {self.var_type_full} v;
}} reference[12];

out {self.interface_name} {{
        {self.var_type_full} {self.var_name};
}} {self.interface_vs_instance};

void main()
{{
        {self.interface_vs_instance}{self.vs_var_ref} = reference[gl_VertexID].v;
}}

[tessellation control shader]
#extension GL_ARB_tessellation_shader : require
layout(vertices = 3) out;

uniform struct S0 {{
        {self.var_type_full} v;
}} reference[12];

in {self.interface_name} {{
        {self.var_type_full} {self.var_name};
}} {self.interface_tcs_instance}[];

out int pass[];

void main()
{{
        const int vertices_in = 3;
        int local_pass = 1;
        for (int i = 0; i < vertices_in; ++i) {{
                int vertex_ID = gl_PrimitiveID * vertices_in + i;
                if ({self.interface_tcs_instance}[i]{self.tcs_var_ref} != reference[vertex_ID].v)
                        local_pass = 0;
        }}
        pass[gl_InvocationID] = local_pass;
        gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);
        gl_TessLevelInner = float[2](1.0, 1.0);
}}

[tessellation evaluation shader]
#extension GL_ARB_tessellation_shader : require
layout(quads) in;

in int pass[];

out vec4 vert_color;

void main()
{{
        const vec4 red = vec4(1, 0, 0, 1);
        const vec4 green = vec4(0, 1, 0, 1);
        vec2[3] position = vec2[3](
                vec2(float(gl_PrimitiveID / 2) - 1.0, float(gl_PrimitiveID % 2) - 1.0),
                vec2(float(gl_PrimitiveID / 2) - 0.0, float(gl_PrimitiveID % 2) - 1.0),
                vec2(float(gl_PrimitiveID / 2) - 1.0, float(gl_PrimitiveID % 2) - 0.0)
        );
        gl_Position = vec4(position[0]
                    + (position[1] - position[0]) * gl_TessCoord[0]
                    + (position[2] - position[0]) * gl_TessCoord[1], 0.0, 1.0);
        vert_color = green;
        if (pass[0] == 0 || pass[1] == 0 || pass[2] == 0) {{
                vert_color = red;
        }}
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
"""

        test = test.format(self = self,
            generator_command = " ".join(sys.argv))

        filename = self.filename()
        dirname = os.path.dirname(filename)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        with open(filename, 'w') as f:
            f.write(test)


def all_tests():
    for type_name in ['float', 'vec2',  'vec3',  'vec4',
                      'mat2', 'mat3', 'mat4',
                      'mat2x3', 'mat2x4', 'mat3x2',
                      'mat3x4', 'mat4x2', 'mat4x3',
                      'int', 'ivec2', 'ivec3', 'ivec4',
                      'uint', 'uvec2', 'uvec3', 'uvec4']:
        for array in [None, 2]:
            yield Test(type_name=type_name, array=array, name=None)
    for var in ['gl_Position', 'gl_PointSize', 'gl_ClipDistance']:
        yield Test(type_name=None, array=None, name=var)

def main():
    for test in all_tests():
        test.generate()
        print(test.filename())


if __name__ == '__main__':
    main()
