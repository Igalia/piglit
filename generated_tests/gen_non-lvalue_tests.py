# coding=utf-8
#
# Copyright © 2012 Intel Corporation
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

import os


class Test(object):
    def __init__(self, type_name, op, usage, shader_target):
        self.type_name = type_name
        self.op = op
        self.usage = usage
        self.shader_target = shader_target

    def filename(self):
        if self.op == "++t":
            name_base = "preincrement"
        elif self.op == "--t":
            name_base = "predecrement"
        elif self.op == "t++":
            name_base = "postincrement"
        elif self.op == "t--":
            name_base = "postdecrement"

        return os.path.join('spec',
                            'glsl-1.10',
                            'compiler',
                            'expressions',
                            '{0}-{1}-non-lvalue-for-{2}.{3}'.format(
                                name_base, self.type_name, self.usage,
                                self.shader_target))

    def generate(self):
        if self.usage == 'assignment':
            test = """/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 *
 * Page 32 (page 38 of the PDF) of the GLSL 1.10 spec says:
 *
 *     "Variables that are built-in types, entire structures, structure
 *     fields, l-values with the field selector ( . ) applied to select
 *     components or swizzles without repeated fields, and l-values
 *     dereferenced with the array subscript operator ( [ ] ) are all
 *     l-values. Other binary or unary expressions, non-dereferenced arrays,
 *     function names, swizzles with repeated fields, and constants cannot be
 *     l-values.  The ternary operator (?:) is also not allowed as an
 *     l-value."
 */
uniform {self.type_name} u;
{mode} vec4 v;

void main()
{{
    {self.type_name} t = u;

    {self.op} = {self.type_name}(v{components});
    {dest} = {var_as_vec4};
}}
"""
        else:
            test = """/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 *
 * Page 32 (page 38 of the PDF) of the GLSL 1.10 spec says:
 *
 *     "Variables that are built-in types, entire structures, structure
 *     fields, l-values with the field selector ( . ) applied to select
 *     components or swizzles without repeated fields, and l-values
 *     dereferenced with the array subscript operator ( [ ] ) are all
 *     l-values. Other binary or unary expressions, non-dereferenced arrays,
 *     function names, swizzles with repeated fields, and constants cannot be
 *     l-values.  The ternary operator (?:) is also not allowed as an
 *     l-value."
 */
uniform {self.type_name} u;
{mode} vec4 v;

void f(out {self.type_name} p)
{{
    p = {self.type_name}(v{components});
}}

void main()
{{
    {self.type_name} t = u;

    f({self.op});
    {dest} = {var_as_vec4};
}}
"""
        if '2' in self.type_name:
            var_as_vec4 = 'vec4(t.xyxy)'
            components = '.xy'
        elif '3' in self.type_name:
            var_as_vec4 = 'vec4(t.xyzx)'
            components = '.xyz'
        elif '4' in self.type_name:
            var_as_vec4 = 'vec4(t)'
            components = ''
        else:
            var_as_vec4 = 'vec4(t)'
            components = '.x'

        if self.shader_target == 'vert':
            dest = "gl_Position"
            mode = 'attribute'
        else:
            mode = 'varying'
            dest = "gl_FragColor"

        test = test.format(self = self,
                           components = components,
                           dest = dest,
                           var_as_vec4 = var_as_vec4,
                           mode = mode)

        filename = self.filename()
	dirname = os.path.dirname(filename)
	if not os.path.exists(dirname):
	    os.makedirs(dirname)
	with open(filename, 'w') as f:
	    f.write(test)


def all_tests():
    for type_name in ['float', 'vec2',  'vec3',  'vec4',
                 'int',   'ivec2', 'ivec3', 'ivec4']:
        for op in ["++t", "--t", "t++", "t--"]:
            for usage in ['assignment', 'out-parameter']:
                          for shader_target in ['vert', 'frag']:
                              yield Test(type_name, op, usage, shader_target)

def main():
    for test in all_tests():
	test.generate()
	print test.filename()


if __name__ == '__main__':
    main()
