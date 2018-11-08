# coding=utf-8
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

""" Module implementing tests for test/all.py and friends

This module only provides tests for native piiglit tests (OpenGL and OpenCL),
it does not provide tests for non native tests that use piglit (oglconfrom,
es3conform, etc)

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import importlib
import os.path

import pytest

MODULES = [
    'all',
    pytest.mark.skipif(not os.path.exists('generated_tests/cl/builtin'),
                       reason='Requires that CL functionality is built')('cl'),
    'cpu',
    'glslparser',
    'gpu',
    'llvmpipe',
    'quick',
    'shader',
]


@pytest.mark.parametrize('name', MODULES)
@pytest.mark.skipif(not os.path.exists('bin'),
                    reason='This test requires the C layer to be built.')
def test_import(name):
    """Test that each built-in module can be imported."""

    importlib.import_module('tests.{}'.format(name))
