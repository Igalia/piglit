# Copyright (c) 2015 Intel Corporation

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

"""Generate tests for glsl 1.10 and 1.20 variable index reads."""

from __future__ import print_function, absolute_import, division
import os
import itertools

from six.moves import range

from templates import template_dir
from modules.utils import lazy_property, safe_makedirs

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))
FS_TEMPLATE = TEMPLATES.get_template('fs.shader_test.mako')
VS_TEMPLATE = TEMPLATES.get_template('vs.shader_test.mako')
DIRNAME = os.path.join('spec', 'glsl-{}', 'execution', 'variable-indexing')


class TestParams(object):
    """Parameters for a single test.

    This is all of the non-formatting logic of the test. Each property is
    wrapped with a lazy_property decorator, which means the data is cached
    after it is calculated once.

    """
    def __init__(self, matrix_dim, array_dim, mode, index_value, col,
                 expect_type, glsl_version):
        self.matrix_dim = matrix_dim
        self.array_dim = array_dim
        self.mode = mode
        self.index_value = index_value
        self.col = col
        self.expect_type = expect_type

        assert glsl_version in [110, 120]
        self.glsl_version = glsl_version

    @lazy_property
    def idx(self):
        if self.array_dim != 0:
            return '[{}]'.format(self.index_value)
        else:
            return ''

    @lazy_property
    def cxr_type(self):
        return 'mat{0}x{0}'.format(self.matrix_dim)

    @lazy_property
    def base_type(self):
        if self.glsl_version == 120:
            return self.cxr_type
        else:
            return 'mat{}'.format(self.matrix_dim)

    @lazy_property
    def type(self):
        if self.array_dim != 0 and self.glsl_version == 120:
            return '{}[{}]'.format(self.base_type, self.array_dim)
        else:
            return self.base_type

    @lazy_property
    def dim(self):
        if self.array_dim != 0 and self.glsl_version == 110:
            return '[{}]'.format(self.array_dim)
        else:
            # XXX: should this be an error?
            return ''

    @lazy_property
    def row(self):
        if self.expect_type == 'float':
            return '[row]'
        else:
            # XXX: Should this be an error?
            return ''

    @lazy_property
    def test_sizes(self):
        if self.array_dim == 0:
            return [1]
        elif self.index_value == 'index':
            return list(range(1, 1 + self.array_dim))
        else:
            return [2]

    @lazy_property
    def test_columns(self):
        if self.col == 'col':
            return list(range(1, 1 + self.matrix_dim))
        else:
            return [2]

    @lazy_property
    def test_rows(self):
        if self.expect_type == 'float':
            return list(range(1, 1 + self.matrix_dim))
        else:
            return [1]

    @lazy_property
    def test_array_dim(self):
        if (self.mode == 'uniform' and
                self.glsl_version == 110 and
                self.array_dim != 0 and
                self.index_value != 'index'):
            return self.index_value + 1
        else:
            return self.array_dim

    @lazy_property
    def varying_comps(self):
        if self.array_dim != 0:
            return 4 + self.matrix_dim**2 * self.array_dim
        else:
            return 4 + self.matrix_dim**2

    @lazy_property
    def formated_version(self):
        return '{:.2f}'.format(float(self.glsl_version) / 100)


def make_fs(name, params):
    """Generate a fragment shader test."""
    dirname = DIRNAME.format(params.formated_version)
    safe_makedirs(dirname)
    with open(os.path.join(dirname, name), 'w') as f:
        f.write(FS_TEMPLATE.render_unicode(params=params))
    print(name)


def make_vs(name, params):
    """Generate a vertex shader test."""
    dirname = DIRNAME.format(params.formated_version)
    safe_makedirs(dirname)
    with open(os.path.join(dirname, name), 'w') as f:
        f.write(VS_TEMPLATE.render_unicode(params=params))
    print(name)


def main():
    """Generate the tests."""
    modes = ['temp', 'uniform', 'varying']
    array_dims = [0, 3]
    matrix_dims = [2, 3, 4]
    glsl_versions = [110, 120]
    iter_ = itertools.product(modes, array_dims, matrix_dims, glsl_versions)
    for mode, array_dim, matrix_dim, glsl_version in iter_:
        if array_dim != 0:
            arr = 'array-'
            idx_text = 'index-'

            # TODO: This can certainly be rolled up into a loop

            make_fs(
                'fs-{mode}-{arr}mat{matrix_dim}-col-row-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 'col', 'float',
                           glsl_version))

            make_fs(
                'fs-{mode}-{arr}mat{matrix_dim}-row-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 1, 'float',
                           glsl_version))

            make_fs(
                'fs-{mode}-{arr}mat{matrix_dim}-col-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 'col',
                           'vec{}'.format(matrix_dim), glsl_version))

            make_fs(
                'fs-{mode}-{arr}mat{matrix_dim}-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 1,
                           'vec{}'.format(matrix_dim), glsl_version))

            make_vs(
                'vs-{mode}-{arr}mat{matrix_dim}-col-row-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 'col', 'float',
                           glsl_version))

            make_vs(
                'vs-{mode}-{arr}mat{matrix_dim}-row-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 1, 'float',
                           glsl_version))

            make_vs(
                'vs-{mode}-{arr}mat{matrix_dim}-col-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 'col',
                           'vec{}'.format(matrix_dim), glsl_version))

            make_vs(
                'vs-{mode}-{arr}mat{matrix_dim}-rd.shader_test'.format(**locals()),
                TestParams(matrix_dim, array_dim, mode, 1, 1,
                           'vec{}'.format(matrix_dim), glsl_version))
        else:
            arr = ''
            idx_text = ''

        make_fs(
            'fs-{mode}-{arr}mat{matrix_dim}-{idx_text}col-row-rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 'col', 'float',
                       glsl_version))

        make_fs(
            'fs-{mode}-{arr}mat{matrix_dim}-{idx_text}row-rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 1, 'float',
                       glsl_version))

        make_fs(
            'fs-{mode}-{arr}mat{matrix_dim}-{idx_text}col-rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 'col',
                       'vec{}'.format(matrix_dim), glsl_version))

        make_fs(
            'fs-{mode}-{arr}mat{matrix_dim}-{idx_text}rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 1,
                       'vec{}'.format(matrix_dim), glsl_version))

        make_vs(
            'vs-{mode}-{arr}mat{matrix_dim}-{idx_text}col-row-rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 'col', 'float',
                       glsl_version))

        make_vs(
            'vs-{mode}-{arr}mat{matrix_dim}-{idx_text}row-rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 1, 'float',
                       glsl_version))

        make_vs(
            'vs-{mode}-{arr}mat{matrix_dim}-{idx_text}col-rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 'col',
                       'vec{}'.format(matrix_dim), glsl_version))

        make_vs(
            'vs-{mode}-{arr}mat{matrix_dim}-{idx_text}rd.shader_test'.format(**locals()),
            TestParams(matrix_dim, array_dim, mode, 'index', 1,
                       'vec{}'.format(matrix_dim), glsl_version))


if __name__ == '__main__':
    main()
