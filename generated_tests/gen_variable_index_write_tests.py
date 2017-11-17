# Copyright (c) 2015-2016 Intel Corporation

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

"""Generate tests for variable index writes.

This creates a TestParams object for each invocation (using a factory to reduce
the number of duplicate objects), and then passes that into a mako template.
The template then formats that information into a shader_test for either the
fragment shader stage or the vertex shader stage.

"""

from __future__ import (
    print_function, absolute_import, division, unicode_literals
)
import copy
import itertools
import os

from six.moves import range  # pylint: disable=redefined-builtin

from modules import utils, glsl
from templates import template_dir

_TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))
_VS_TEMPLATE = _TEMPLATES.get_template('vs.shader_test.mako')
_FS_TEMPLATE = _TEMPLATES.get_template('fs.shader_test.mako')
_DIRNAME = os.path.join('spec', 'glsl-{}', 'execution', 'variable-indexing')


class TestParams(object):
    """Object representing all of the parameters of a single test instance.

    Provides all of the values using lazy properties, which tie in with the
    ParamsFactory to store all of the values, speeding up run times.

    """
    def __init__(self, mode, array_dim, matrix_dim, index_value, col,
                 value_type, glsl_version):
        # pylint: disable=too-many-arguments
        assert array_dim in [0, 3]
        assert matrix_dim in [2, 3, 4]

        self.mode = mode
        self.array_dim = array_dim
        self.matrix_dim = matrix_dim
        self.index_value = index_value
        self.col = '[{}]'.format(col)
        self.value_type = value_type
        self.version = glsl.Version(glsl_version)

    @utils.lazy_property
    def varying_comps(self):
        if self.array_dim != 0:
            return self.matrix_dim**2 * self.array_dim
        else:
            return self.matrix_dim**2

    @utils.lazy_property
    def base_type(self):
        if int(self.version) >= 120:
            return 'mat{0}x{0}'.format(self.matrix_dim)
        else:
            return 'mat{}'.format(self.matrix_dim)

    @utils.lazy_property
    def type(self):
        if self.array_dim != 0 and int(self.version) >= 120:
            return '{}[{}]'.format(self.base_type, self.array_dim)
        else:
            return self.base_type

    @utils.lazy_property
    def dim(self):
        if self.array_dim != 0 and int(self.version) < 120:
            return '[{}]'.format(self.array_dim)
        else:
            return ''

    @utils.lazy_property
    def row(self):
        if self.value_type == 'float':
            return '[row]'
        else:
            return ''

    @utils.lazy_property
    def idx(self):
        if self.array_dim != 0:
            return '[{}]'.format(self.index_value)
        else:
            return ''

    @utils.lazy_property
    def test_vec(self):
        if self.matrix_dim == 2:
            return ["0.803161418975390", "0.852987140792140"]
        elif self.matrix_dim == 3:
            return ["0.681652305322399", "0.210426138878113",
                    "0.185916924650237"]
        elif self.matrix_dim == 4:
            return ["0.0394868046587045", "0.8922408276905568",
                    "0.3337495624366961", "0.8732295730825839"]

    @utils.lazy_property
    def test_exp(self):
        if self.matrix_dim == 2:
            return ["0.708718134966688", "1.452243795483797"]
        elif self.matrix_dim == 3:
            return ["0.610649606928364", "0.711906885823636",
                    "0.312244778977868"]
        elif self.matrix_dim == 4:
            return ["1.03935908892461", "1.18846180713529", "1.10078681232072",
                    "1.72434439561820"]

    @utils.lazy_property
    def test_mat(self):
        if self.matrix_dim == 2:
            return [["0.241498998195656", "0.861223395812970"],
                    ["0.603473877011433", "0.891622340451180"]]
        elif self.matrix_dim == 3:
            return [
                ["0.493944462129466", "0.722190133917966", "0.239853948232558"],
                ["0.550143078409278", "0.591962645398579", "0.467616286531193"],
                ["0.850846377186973", "0.511303112962423", "0.270815003356504"]
            ]
        elif self.matrix_dim == 4:
            return [["0.922040144261674", "0.158053783109488",
                     "0.357016429866574", "0.836368810383957"],
                    ["0.560251913703792", "0.171634921595771",
                     "0.602494709909111", "0.693273570571311"],
                    ["0.350720358904176", "0.912192627475775",
                     "0.688544081259531", "0.913891056231967"],
                    ["0.442058176039301", "0.829835836794679",
                     "0.365674411003021", "0.879197364462782"]]

    @utils.lazy_property
    def test_sizes(self):
        if self.array_dim == 0:
            return [1]
        elif self.index_value == 'index':
            return list(range(1, self.array_dim + 1))
        else:
            return [2]

    @utils.lazy_property
    def test_columns(self):
        if self.col == '[col]':
            return list(range(1, self.matrix_dim + 1))
        return [2]

    @utils.lazy_property
    def test_rows(self):
        if self.value_type == 'float':
            return list(range(1, self.matrix_dim + 1))
        return [1]

    @utils.lazy_property
    def test_type(self):
        # shader_runner always uses matDxD format
        return 'mat{0}x{0}'.format(self.matrix_dim)

    def test_matrix(self, column, row):
        """Generate the matrix used in a test section.

        This will take the matrix used by the test, and replace specific values
        with sentinal values, and return the matrix as a string.

        """
        bad = ['666.0', '777.0', '888.0', '999.0']
        mat = copy.deepcopy(self.test_mat)

        if self.value_type == 'float':
            mat[column][row] = bad[0]
        else:
            mat[column] = bad[0:self.matrix_dim]

        ret = ''
        for c in mat:
            ret += ' '.join(c)
            ret += ' '
        return ret

    @utils.lazy_property
    def formated_version(self):
        # Note: GLSLVersion.float() does division by 100
        return '{:.2f}'.format(float(self.version))


class ParamsFactory(object):  # pylint: disable=too-few-public-methods
    """A factory class that provides TestParam objects.

    This cuts the number of new objects created by roughly 3/5.

    """
    def __init__(self):
        self.__stored = {}

    def get(self, *args):
        args = tuple(args)
        try:
            return self.__stored[args]
        except KeyError:
            params = TestParams(*args)
            self.__stored[args] = params
            return params


def make_vs(name, params):
    """Create a vertex shader test."""
    dirname = _DIRNAME.format(params.formated_version)
    utils.safe_makedirs(dirname)
    with open(os.path.join(dirname, name), 'w') as f:
        f.write(_VS_TEMPLATE.render_unicode(params=params))
    print(name)


def make_fs(name, params):
    """Create a fragment shader test."""
    dirname = _DIRNAME.format(params.formated_version)
    utils.safe_makedirs(dirname)
    with open(os.path.join(dirname, name), 'w') as f:
        f.write(_FS_TEMPLATE.render_unicode(params=params))
    print(name)


def main():
    """The main function."""
    # Various choices that can be put together to produce a single test.
    iter_ = itertools.product(
        ['110', '120'],       # GLSL versions
        [0, 3],               # Array dimensions
        [2, 3, 4],            # Matrix dimensions
        ['varying', 'temp'],  # modes
        ['col', 1],           # columns
        ['fs', 'vs'],         # shader stages
    )

    factory = ParamsFactory()

    # This can be filled in to produce the file name for the test.
    # Note that idx, col, row, and arr will need to have a '-' added to the end
    # of the value if it is not empty
    name = '{stage}-{mode}-{arr}mat{matrix_dim}-{idx}{col}{row}wr.shader_test'

    for v, a, d, m, c, s in iter_:
        for t in ['float', 'vec{}'.format(d)]:
            if s == 'vs':
                func = make_vs
            elif s == 'fs':
                if m == 'varying':
                    # Fragment shaders cannot write varyings
                    continue
                func = make_fs

            if a != 0:
                arr = 'array-'

                func(
                    name.format(stage=s,
                                mode=m,
                                matrix_dim=d,
                                arr=arr,
                                idx='',
                                col='col-' if c == 'col' else '',
                                row='row-' if t == 'float' else ''),
                    factory.get(m, a, d, 1, c, t, v))
            else:
                arr = ''

            func(
                name.format(stage=s,
                            mode=m,
                            matrix_dim=d,
                            arr=arr,
                            idx='index-' if a != 0 else '',
                            col='col-' if c == 'col' else '',
                            row='row-' if t == 'float' else ''),
                factory.get(m, a, d, 'index', c, t, v))


if __name__ == '__main__':
    main()
