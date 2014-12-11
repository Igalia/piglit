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

""" Generate glsl 1.20 outerproduct tests """

from __future__ import print_function
import os
import itertools
import collections

from templates import template_file

TEMPLATE = template_file(os.path.splitext(os.path.basename(__file__))[0],
                         'template.shader_test.mako')

Parameters = collections.namedtuple(
    'Paramters', ['columns', 'rows', 'vec_type', 'matrix'])


def main():
    """ Generate tests """
    dirname = os.path.join('spec', 'glsl-1.20', 'execution')
    if not os.path.exists(dirname):
        try:
            os.makedirs(dirname)
        except OSError as e:
            if e.errno == 17:  # file exists
                pass
            raise

    for c, r in itertools.product(xrange(2, 5), repeat=2):
        vecs = [
            Parameters(c, r, 'vec', 'mat{0}x{1}'.format(r, c)),
            Parameters(c, r, 'ivec', 'mat{0}x{1}'.format(r, c))
        ]
        if r == c:
            vecs.extend([
                Parameters(c, r, 'vec', 'mat{0}'.format(r)),
                Parameters(c, r, 'ivec', 'mat{0}'.format(r))
            ])

        stages = ['vs', 'fs']
        types = ['const', 'uniform']
        for shader, type_, params in itertools.product(stages, types, vecs):
            name = os.path.join(
                dirname,
                '{shader}-outerProduct-{type}{mat}{vec}.shader_test'.format(
                    shader=shader,
                    type='const-' if type_ == 'const' else '',
                    mat=params.matrix,
                    vec='-ivec' if params.vec_type == 'ivec' else ''))

            print(name)
            with open(name, 'w+') as f:
                f.write(TEMPLATE.render_unicode(params=params,
                                                type=type_,
                                                shader=shader))


if __name__ == '__main__':
    main()
