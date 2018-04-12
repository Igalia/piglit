# coding=utf-8
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

"""Generate tests for running the generators.

This needs to be compatible with both python2 and python3.

Tests that take more than ~10 seconds should be marked as "slow" and tests that
take more than ~30 seconds should be marked as "very_slow".
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import importlib
try:
    import mock
except ImportError:
    from unittest import mock

import pytest


@pytest.mark.parametrize('name', [
    'gen_builtin_packing_tests',
    'gen_builtin_uniform_tests_fp64',
    'gen_cl_common_builtins',
    'gen_cl_int_builtins',
    'gen_cl_math_builtins',
    'gen_cl_relational_builtins',
    'gen_cl_shuffle2_builtins',
    'gen_cl_shuffle_builtins',
    'gen_cl_store_tests',
    'gen_cl_vload_tests',
    'gen_cl_vstore_tests',
    'gen_const_builtin_equal_tests',
    'gen_constant_array_size_tests_fp64',
    'gen_conversion',
    'gen_extensions_defined',
    'gen_flat_interpolation_qualifier',
    'gen_inout_fp64',
    'gen_interpolation_tests',
    'gen_non-lvalue_tests',
    'gen_outerproduct_invalid_params',
    'gen_outerproduct_tests',
    'gen_shader_bit_encoding_tests',
    'gen_shader_framebuffer_fetch_tests',
    'gen_shader_image_load_store_tests',
    'gen_shader_image_nv_image_formats_tests',
    'gen_shader_intel_conservative_rasterization',
    'gen_shader_precision_tests',
    'gen_tcs_input_tests',
    'gen_tes_input_tests',
    'gen_texture_lod_tests',
    'gen_texture_query_lod_tests',
    'gen_uniform_initializer_tests',
    'gen_variable_index_read_tests',
    'gen_variable_index_write_tests',
    'gen_vp_tex',
    'interpolation-qualifier-built-in-variable',
    pytest.mark.slow('gen_builtin_uniform_tests'),
    pytest.mark.slow('gen_builtin_uniform_spirv_tests'),
    pytest.mark.slow('gen_constant_array_size_tests'),
    pytest.mark.very_slow('gen_vs_in_fp64'),
])
def test_generators(name, tmpdir):
    """Teat each generator."""
    mod = importlib.import_module(name)
    tmpdir.chdir()

    # Some tests do checks for sys.argv, so mock that out. Also mock out
    # print since we don't want a giant list of tests printed on an error.
    with mock.patch('sys.argv', [name]), \
            mock.patch.object(mod, 'print', mock.Mock(), create=True):
        mod.main()
