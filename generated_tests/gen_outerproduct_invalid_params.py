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

from __future__ import print_function
import os
import textwrap
import mako.template


def main():
    """ Generate tests """
    template = mako.template.Template(textwrap.dedent("""
    /* [config]
     * expect_result: fail
     * glsl_version: 1.20
     * [end config]
     */
    #version 120
    void main() {
        gl_Position = vec4(0);
        outerProduct(${type}(0), ${type}(0));
    }
    """))

    dirname = os.path.join('spec', 'glsl-1.20', 'compiler',
                           'built-in-functions')
    if not os.path.exists(dirname):
        os.makedirs(dirname)

    for type_ in ['int', 'float', 'bool', 'bvec2', 'bvec3', 'bvec4', 'mat2',
                  'mat2x2', 'mat2x3', 'mat2x4', 'mat3', 'mat3x2', 'mat3x3',
                  'mat3x4', 'mat4', 'mat4x2', 'mat4x3', 'mat4x4']:
        name = os.path.join(dirname, 'outerProduct-{0}.vert'.format(type_))
        print(name)
        with open(name, 'w+') as f:
            f.write(template.render_unicode(type=type_))


if __name__ == '__main__':
    main()
