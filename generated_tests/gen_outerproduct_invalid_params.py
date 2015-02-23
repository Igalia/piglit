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

from __future__ import print_function, division, absolute_import
import os

from templates import template_file
from modules import utils

TEMPLATE = template_file(os.path.basename(os.path.splitext(__file__)[0]),
                         'template.vert.mako')


def main():
    """ Generate tests """
    dirname = os.path.join('spec', 'glsl-1.20', 'compiler',
                           'built-in-functions')
    utils.safe_makedirs(dirname)

    for type_ in ['int', 'float', 'bool', 'bvec2', 'bvec3', 'bvec4', 'mat2',
                  'mat2x2', 'mat2x3', 'mat2x4', 'mat3', 'mat3x2', 'mat3x3',
                  'mat3x4', 'mat4', 'mat4x2', 'mat4x3', 'mat4x4']:
        name = os.path.join(dirname, 'outerProduct-{0}.vert'.format(type_))
        print(name)
        with open(name, 'w+') as f:
            f.write(TEMPLATE.render_unicode(type=type_))


if __name__ == '__main__':
    main()
