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

"""Tests for generated_tests/modules/glsl.py."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools
import operator
import os
import sys

import nose.tools as nt
import six

# Add <piglit root>/generated_tests to the module path, this allows it to be
# imported for testing.
sys.path.insert(0, os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', 'generated_tests')))

# pylint can't figure out the sys.path manipulation.
from modules import glsl  # pylint: disable=import-error
from .. import utils

# pylint: disable=no-self-use,invalid-name

_GLSL = [
    '110',
    '120',
    '130',
    '140',
    '150',
    '330',
    '400',
    '410',
    '420',
    '430',
    '440',
    '450',
]

_GLSLES = [
    '100',
    '300 es',
    '310 es',
    '320 es',
]


def test_factory_glsl():
    """generated_tests.modules.glsl.Version: provides a GLSLVersion."""
    test = glsl.Version('110')
    nt.assert_is_instance(test, glsl.GLSLVersion)


def test_factory_glsles():
    """generated_tests.modules.glsl.Version: provides a GLSLESVersion."""
    test = glsl.Version('100')
    nt.assert_is_instance(test, glsl.GLSLESVersion)


def test_factory_cache():
    """generated_tests.modules.glsl.Version: caches objects."""
    test1 = glsl.Version('100')
    test2 = glsl.Version('100')
    nt.assert_is(test1, test2)


class TestCompare(object):
    """Comparison tests for GLSLESVersion and GLSLVersion."""
    _operators = [
        ('<', operator.lt),
        ('<=', operator.le),
        ('==', operator.eq),
        ('!=', operator.ne),
        ('>=', operator.ge),
        ('>', operator.gt),
    ]

    @utils.nose_generator
    def test_glsl_glsl(self):
        """Test GLSLVersion <cmp> GLSLVersion."""
        def expected(first, second, op):
            return op(int(first), int(second))

        def test(first, second, op):
            nt.eq_(op(glsl.Version(first), glsl.Version(second)),
                   expected(first, second, op))

        desc = 'generated_tests.modules.glsl.GLSLVersion: {} {} {}'

        for ver1, ver2 in itertools.combinations(_GLSL, 2):
            for name, op in self._operators:
                test.description = desc.format(ver1, name, ver2)
                yield test, ver1, ver2, op

    @utils.nose_generator
    def test_glsles_glsles(self):
        """Test GLSLESVersion <cmp> GLSLESVersion."""
        def expected(first, second, op):
            # use the slice to drop " es" if it exists
            return op(int(first[:3]), int(second[:3]))

        def test(first, second, op):
            nt.eq_(op(glsl.Version(first), glsl.Version(second)),
                   expected(first, second, op))

        desc = 'generated_tests.modules.glsl.GLSLESVersion: {} {} {}'

        for ver1, ver2 in itertools.combinations(_GLSLES, 2):
            for name, op in self._operators:
                test.description = desc.format(ver1, name, ver2)
                yield test, ver1, ver2, op

    @nt.raises(TypeError)
    def test_glsl_glsles(self):
        """generated_tests.modules.glsl: GLSLVersion <cmp> GLSLESVersion."""
        return glsl.Version('110') < glsl.Version('100')

    @nt.raises(TypeError)
    def test_glsles_glsl(self):
        """generated_tests.modules.glsl: GLSLESVersion <cmp> GLSLVersion."""
        return glsl.Version('100') < glsl.Version('110')

    @utils.nose_generator
    def test_glsl_int(self):
        """Test GLSLVersion <cmp> GLSLVersion."""
        def expected(first, second, op):
            return op(int(first), int(second))

        def test(first, second, op, expect):
            nt.eq_(op(first, second), expect)

        desc = 'generated_tests.modules.glsl.GLSLVersion: {} {} {}'

        for ver1, ver2 in itertools.combinations(_GLSL, 2):
            for name, op in self._operators:
                test.description = desc.format(
                    'GLSLVersion({})'.format(ver1),
                    name,
                    'int({})'.format(ver2))
                yield (test, glsl.Version(ver1), int(ver2), op,
                       expected(ver1, ver2, op))

                test.description = desc.format(
                    'int({})'.format(ver1),
                    name,
                    'GLSLVersion({})'.format(ver2))
                yield (test, int(ver1), glsl.Version(ver2), op,
                       expected(ver1, ver2, op))

    @utils.nose_generator
    def test_glsl_float(self):
        """Test GLSLVersion <cmp> GLSLVersion."""
        def expected(first, second, op):
            return op(float(first) / 100, float(second) / 100)

        def test(first, second, op, expect):
            nt.eq_(op(first, second), expect)

        desc = 'generated_tests.modules.glsl.GLSLVersion: {} {} {}'

        for ver1, ver2 in itertools.combinations(_GLSL, 2):
            for name, op in self._operators:
                test.description = desc.format(
                    'GLSLVersion({})'.format(ver1),
                    name,
                    'float({})'.format(ver2))
                yield (test, glsl.Version(ver1), float(ver2) / 100, op,
                       expected(ver1, ver2, op))

                test.description = desc.format(
                    'float({})'.format(ver1),
                    name,
                    'GLSLVersion({})'.format(ver2))
                yield (test, float(ver1) / 100, glsl.Version(ver2), op,
                       expected(ver1, ver2, op))

    @utils.nose_generator
    def test_glsles_int(self):
        """Test GLSLESVersion <cmp> GLSLESVersion."""
        def expected(first, second, op):
            return op(int(first[:3]), int(second[:3]))

        def test(first, second, op, expect):
            nt.eq_(op(first, second), expect)

        desc = 'generated_tests.modules.glsl.GLSLESVersion: {} {} {}'

        for ver1, ver2 in itertools.combinations(_GLSLES, 2):
            for name, op in self._operators:
                test.description = desc.format(
                    'GLSLESVersion({})'.format(ver1),
                    name,
                    'int({})'.format(ver2))
                # Slice to avoid calling int on '300 es'
                yield (test, glsl.Version(ver1), int(ver2[:3]), op,
                       expected(ver1, ver2, op))

                test.description = desc.format(
                    'int({})'.format(ver1),
                    name,
                    'GLSLESVersion({})'.format(ver2))
                # Slice to avoid calling int on '300 es'
                yield (test, int(ver1[:3]), glsl.Version(ver2), op,
                       expected(ver1, ver2, op))

    @utils.nose_generator
    def test_glsles_float(self):
        """Test GLSLESVersion <cmp> GLSLESVersion."""
        def expected(first, second, op):
            return op(float(first[:3]) / 100, float(second[:3]) / 100)

        def test(first, second, op, expect):
            nt.eq_(op(first, second), expect)

        desc = 'generated_tests.modules.glsl.GLSLESVersion: {} {} {}'

        for ver1, ver2 in itertools.combinations(_GLSLES, 2):
            for name, op in self._operators:
                test.description = desc.format(
                    'GLSLESVersion({})'.format(ver1),
                    name,
                    'float({})'.format(ver2))
                # Slice to avoid calling float on '300 es'
                yield (test, glsl.Version(ver1), float(ver2[:3]) / 100, op,
                       expected(ver1, ver2, op))

                test.description = desc.format(
                    'float({})'.format(ver1),
                    name,
                    'GLSLESVersion({})'.format(ver2))
                # Slice to avoid calling float on '300 es'
                yield (test, float(ver1[:3]) / 100, glsl.Version(ver2), op,
                       expected(ver1, ver2, op))


def test_GLSLVersion_str():
    """generated_tests.modules.glsl.GLSLVersion: str()"""
    nt.eq_(six.text_type(glsl.Version('110')), '110')


def test_GLSLESVersion_str():
    """generated_tests.modules.glsl.GLSLESVersion: str()"""
    nt.eq_(six.text_type(glsl.Version('100')), '100')


def test_GLSLVersion_int():
    """generated_tests.modules.glsl.GLSLVersion: int()"""
    nt.eq_(int(glsl.Version('110')), 110)


def test_GLSLESVersion_int():
    """generated_tests.modules.glsl.GLSLESVersion: int()"""
    nt.eq_(int(glsl.Version('100')), 100)


def test_GLSLVersion_float():
    """generated_tests.modules.glsl.GLSLVersion: float()"""
    nt.eq_(float(glsl.Version('110')), 1.10)


def test_GLSLESVersion_float():
    """generated_tests.modules.glsl.GLSLESVersion: float()"""
    nt.eq_(float(glsl.Version('100')), 1.00)


def test_GLSLVersion_print_float():
    """generated_tests.modules.glsl.GLSLVersion: print_float()"""
    nt.eq_(glsl.Version('110').print_float(), '1.10')


def test_GLSLESVersion_print_float():
    """generated_tests.modules.glsl.GLSLESVersion: print_float()"""
    nt.eq_(glsl.Version('100').print_float(), '1.00')


def test_GLSLESVersion_print_float_es():
    """generated_tests.modules.glsl.GLSLESVersion: print_float() (es version)"""
    nt.eq_(glsl.Version('300 es').print_float(), '3.00 es')
