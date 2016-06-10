# encoding=utf-8
# Copyright © 2016 Intel Corporation

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

"""Tests from generated_tests/modules/types.py"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools
import os
import sys

import nose.tools as nt

from .. import utils

# Add <piglit root>/generated_tests to the module path, this allows it to be
# imported for testing.
sys.path.insert(0, os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', 'generated_tests')))

# pylint can't figure out the sys.path manipulation.
from modules import types  # pylint: disable=import-error


@utils.nose.Skip(not __debug__, 'Test requires debug asserts')
def test_container_is_type_assert():
    """modules.types.Container: Only accept one of is_scalar or is_vector or is_matrix"""
    for s, v, m in itertools.product([True, False], repeat=3):
        # Don't test the valid case
        if [s, v, m].count(True) == 0:
            continue

        with nt.assert_raises(AssertionError):
            types.Container('foo', is_scalar=s, is_vector=v, is_matrix=m,
                            contains=types.FLOAT)


def test_matrix_is_square():
    """modules.types.Matrix.square: works for square matricies"""
    for mat in [types.MAT2, types.DMAT3X3]:
        nt.eq_(mat.square, True)


def test_matrix_is_not_square():
    """modules.types.Matrix.square: works for non-square matricies"""
    nt.eq_(types.MAT2X4.square, False)


@utils.nose.Skip(not __debug__, 'Test requires debug asserts')
@nt.raises(AssertionError)
def test_type_int_float_assert():
    """modules.types.Type: only integer or floating can be passed."""
    types.Type('foo', integer=True, floating=True, size=32)


@utils.nose.Skip(not __debug__, 'Test requires debug asserts')
@nt.raises(AssertionError)
def test_type_float_signed_assert():
    """modules.types.Type: floating types must be signed."""
    types.Type('foo', floating=True, signed=False, size=32)
