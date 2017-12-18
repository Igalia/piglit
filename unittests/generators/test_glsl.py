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

import pytest
import six

# pylint can't figure out the sys.path manipulation.
from modules import glsl  # pylint: disable=import-error

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

_OPERATORS = [
    operator.lt,
    operator.le,
    operator.eq,
    operator.ne,
    operator.ge,
    operator.gt,
]


def test_factory_glsl():
    """generated_tests.modules.glsl.Version: provides a GLSLVersion."""
    test = glsl.Version('110')
    assert isinstance(test, glsl.GLSLVersion)


def test_factory_glsles():
    """generated_tests.modules.glsl.Version: provides a GLSLESVersion."""
    test = glsl.Version('100')
    assert isinstance(test, glsl.GLSLESVersion)


def test_factory_cache():
    """generated_tests.modules.glsl.Version: caches objects."""
    test1 = glsl.Version('100')
    test2 = glsl.Version('100')
    assert test1 is test2


@pytest.mark.parametrize("op,first,second",
                         itertools.product(_OPERATORS, _GLSL, _GLSL))
def test_compare_glsl_to_glsl(op, first, second):
    """Test GLSLVersion <cmp> GLSLVersion."""
    actual = op(glsl.Version(first), glsl.Version(second))
    assert actual == op(int(first), int(second))


@pytest.mark.parametrize("op,first,second",
                         itertools.product(_OPERATORS, _GLSLES, _GLSLES))
def test_compare_glsles_to_glsles(op, first, second):
    """Test GLSLESVersion <cmp> GLSLESVersion."""
    actual = op(glsl.Version(first), glsl.Version(second))
    assert actual == op(int(first[:3]), int(second[:3]))


@pytest.mark.raises(exception=TypeError)
def test_glsl_glsles_exception():
    """generated_tests.modules.glsl: GLSLVersion <cmp> GLSLESVersion."""
    return glsl.Version('110') < glsl.Version('100')


@pytest.mark.raises(exception=TypeError)
def test_glsles_glsl_exception():
    """generated_tests.modules.glsl: GLSLESVersion <cmp> GLSLVersion."""
    return glsl.Version('100') < glsl.Version('110')


@pytest.mark.parametrize("op,first,second",
                         itertools.product(_OPERATORS, _GLSL, _GLSL))
def test_compare_glsl_to_int(op, first, second):
    """Test GLSLVersion <cmp> GLSLVersion."""
    actual = op(glsl.Version(first), int(second))
    assert actual == op(int(first), int(second))


@pytest.mark.parametrize("op,first,second",
                         itertools.product(_OPERATORS, _GLSLES, _GLSLES))
def test_compare_glsles_to_int(op, first, second):
    """Test GLSLESVersion <cmp> GLSLESVersion."""
    actual = op(glsl.Version(first), int(second[:3]))
    assert actual == op(int(first[:3]), int(second[:3]))


@pytest.mark.parametrize("op,first,second",
                         itertools.product(_OPERATORS, _GLSL, _GLSL))
def test_compare_glsl_to_float(op, first, second):
    """Test GLSLVersion <cmp> GLSLVersion."""
    actual = op(glsl.Version(first), float(second) / 100)
    expected = op(float(first), float(second))
    assert actual == expected


@pytest.mark.parametrize("op,first,second",
                         itertools.product(_OPERATORS, _GLSLES, _GLSLES))
def test_compare_glsles_to_float(op, first, second):
    """Test GLSLESVersion <cmp> GLSLESVersion."""
    actual = op(glsl.Version(first), float(second[:3]) / 100)
    assert actual == op(float(first[:3]), float(second[:3]))




def test_GLSLVersion_str():
    """generated_tests.modules.glsl.GLSLVersion: str()"""
    assert six.text_type(glsl.Version('110')) == '110'


def test_GLSLESVersion_str():
    """generated_tests.modules.glsl.GLSLESVersion: str()"""
    assert six.text_type(glsl.Version('100')) == '100'


def test_GLSLVersion_int():
    """generated_tests.modules.glsl.GLSLVersion: int()"""
    assert int(glsl.Version('110')) == 110


def test_GLSLESVersion_int():
    """generated_tests.modules.glsl.GLSLESVersion: int()"""
    assert int(glsl.Version('100')) == 100


def test_GLSLVersion_float():
    """generated_tests.modules.glsl.GLSLVersion: float()"""
    assert float(glsl.Version('110')) == 1.10


def test_GLSLESVersion_float():
    """generated_tests.modules.glsl.GLSLESVersion: float()"""
    assert float(glsl.Version('100')) == 1.00


def test_GLSLVersion_print_float():
    """generated_tests.modules.glsl.GLSLVersion: print_float()"""
    assert glsl.Version('110').print_float() == '1.10'


def test_GLSLESVersion_print_float():
    """generated_tests.modules.glsl.GLSLESVersion: print_float()"""
    assert glsl.Version('100').print_float() == '1.00'


def test_GLSLESVersion_print_float_es():
    """generated_tests.modules.glsl.GLSLESVersion: print_float() (es version)"""
    assert glsl.Version('300 es').print_float() == '3.00 es'


class TestMinVersion_for_stage(object):
    """Tests for generated_tests.modules.glsl.MinVersion.for_stage.

    Each test covers the requested < required and requested == required. If
    it's possible it also covers requested > required.

    """
    def _test(self, stage, version, expected):
        assert glsl.MinVersion.for_stage(stage, version) == expected

    def test_opengl_frag(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: FS (OpenGL)"""
        self._test('frag', glsl.Version('150'), glsl.Version('150'))
        self._test('frag', glsl.Version('110'), glsl.Version('110'))

    def test_opengl_vert(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: VS (OpenGL)"""
        self._test('vert', glsl.Version('150'), glsl.Version('150'))
        self._test('vert', glsl.Version('110'), glsl.Version('110'))

    def test_opengl_geom(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: GS (OpenGL)"""
        self._test('geom', glsl.Version('330'), glsl.Version('330'))
        self._test('geom', glsl.Version('110'), glsl.Version('150'))
        self._test('geom', glsl.Version('150'), glsl.Version('150'))

    def test_opengl_tesc(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: TCS (OpenGL)"""
        self._test('tesc', glsl.Version('410'), glsl.Version('410'))
        self._test('tesc', glsl.Version('400'), glsl.Version('400'))
        self._test('tesc', glsl.Version('140'), glsl.Version('400'))

    def test_opengl_tese(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: TES (OpenGL)"""
        self._test('tese', glsl.Version('410'), glsl.Version('410'))
        self._test('tese', glsl.Version('400'), glsl.Version('400'))
        self._test('tese', glsl.Version('140'), glsl.Version('400'))

    def test_opengl_comp(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: CS (OpenGL)"""
        self._test('comp', glsl.Version('440'), glsl.Version('440'))
        self._test('comp', glsl.Version('430'), glsl.Version('430'))
        self._test('comp', glsl.Version('140'), glsl.Version('430'))

    def test_opengles_frag(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: FS (OpenGL ES)"""
        self._test('frag', glsl.Version('300 es'), glsl.Version('300 es'))
        self._test('frag', glsl.Version('100'), glsl.Version('100'))

    def test_opengles_vert(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: VS (OpenGL ES)"""
        self._test('vert', glsl.Version('320 es'), glsl.Version('320 es'))
        self._test('vert', glsl.Version('100'), glsl.Version('100'))

    def test_opengles_geom(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: GS (OpenGL ES)"""
        self._test('geom', glsl.Version('100'), glsl.Version('320 es'))
        self._test('geom', glsl.Version('320 es'), glsl.Version('320 es'))

    def test_opengles_tesc(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: TCS (OpenGL ES)"""
        self._test('tesc', glsl.Version('320 es'), glsl.Version('320 es'))
        self._test('tesc', glsl.Version('100'), glsl.Version('320 es'))

    def test_opengles_tese(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: TES (OpenGL ES)"""
        self._test('tese', glsl.Version('320 es'), glsl.Version('320 es'))
        self._test('tese', glsl.Version('100'), glsl.Version('320 es'))

    def test_opengles_comp(self):
        """generated_tests.modules.glsl.MinVersion.for_stage: TES (OpenGL ES)"""
        self._test('comp', glsl.Version('320 es'), glsl.Version('320 es'))
        self._test('comp', glsl.Version('100'), glsl.Version('310 es'))
        self._test('comp', glsl.Version('310 es'), glsl.Version('310 es'))


class TestMinVersion_for_stage_with_ext(object):
    """Tests for generated_tests.modules.glsl.MinVersion.for_stage_with_ext."""
    def _test(self, stage, version, expected):
        ver, ext = glsl.MinVersion.for_stage_with_ext(stage, version)
        assert (ver, ext) == expected

    def test_opengl_frag(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        FS (OpenGL)"""
        self._test('frag', glsl.Version('150'), (glsl.Version('150'), None))
        self._test('frag', glsl.Version('110'), (glsl.Version('110'), None))

    def test_opengl_vert(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        VS (OpenGL)"""
        self._test('vert', glsl.Version('150'), (glsl.Version('150'), None))
        self._test('vert', glsl.Version('110'), (glsl.Version('110'), None))

    def test_opengl_geom(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        GS (OpenGL)"""
        self._test('geom', glsl.Version('330'), (glsl.Version('330'), None))
        self._test('geom', glsl.Version('110'), (glsl.Version('150'), None))
        self._test('geom', glsl.Version('150'), (glsl.Version('150'), None))

    def test_opengl_tesc(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        TCS (OpenGL)"""
        self._test('tesc', glsl.Version('410'), (glsl.Version('410'), None))
        self._test('tesc', glsl.Version('140'),
                   (glsl.Version('140'), 'GL_ARB_tessellation_shader'))
        self._test('tesc', glsl.Version('110'),
                   (glsl.Version('140'), 'GL_ARB_tessellation_shader'))

    def test_opengl_tese(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        TES (OpenGL)"""
        self._test('tese', glsl.Version('410'), (glsl.Version('410'), None))
        self._test('tese', glsl.Version('140'),
                   (glsl.Version('140'), 'GL_ARB_tessellation_shader'))
        self._test('tese', glsl.Version('110'),
                   (glsl.Version('140'), 'GL_ARB_tessellation_shader'))

    def test_opengl_comp(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        CS (OpenGL)"""
        self._test('comp', glsl.Version('430'), (glsl.Version('430'), None))
        self._test('comp', glsl.Version('140'),
                   (glsl.Version('140'), 'GL_ARB_compute_shader'))
        self._test('comp', glsl.Version('110'),
                   (glsl.Version('140'), 'GL_ARB_compute_shader'))

    def test_opengles_frag(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        FS (OpenGL ES)"""
        self._test('frag', glsl.Version('300 es'),
                   (glsl.Version('300 es'), None))
        self._test('frag', glsl.Version('100'), (glsl.Version('100'), None))

    def test_opengles_vert(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        VS (OpenGL ES)"""
        self._test('vert', glsl.Version('300 es'),
                   (glsl.Version('300 es'), None))
        self._test('vert', glsl.Version('100'), (glsl.Version('100'), None))

    def test_opengles_geom(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        GS (OpenGL ES)"""
        self._test('geom', glsl.Version('100'),
                   (glsl.Version('310 es'), 'GL_OES_geometry_shader'))
        self._test('geom', glsl.Version('310 es'),
                   (glsl.Version('310 es'), 'GL_OES_geometry_shader'))
        self._test('geom', glsl.Version('320 es'),
                   (glsl.Version('320 es'), None))

    def test_opengles_tesc(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        TCS (OpenGL ES)"""
        self._test('tesc', glsl.Version('320 es'),
                   (glsl.Version('320 es'), None))
        self._test('tesc', glsl.Version('310 es'),
                   (glsl.Version('310 es'), 'GL_OES_tessellation_shader'))
        self._test('tesc', glsl.Version('100'),
                   (glsl.Version('310 es'), 'GL_OES_tessellation_shader'))

    def test_opengles_tese(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        TES (OpenGL ES)"""
        self._test('tese', glsl.Version('320 es'),
                   (glsl.Version('320 es'), None))
        self._test('tese', glsl.Version('310 es'),
                   (glsl.Version('310 es'), 'GL_OES_tessellation_shader'))
        self._test('tese', glsl.Version('100'),
                   (glsl.Version('310 es'), 'GL_OES_tessellation_shader'))

    def test_opengles_comp(self):
        """generated_tests.modules.glsl.MinVersion.for_stage_with_ext:
        TES (OpenGL ES)"""
        self._test('comp', glsl.Version('320 es'),
                   (glsl.Version('320 es'), None))
        self._test('comp', glsl.Version('100'), (glsl.Version('310 es'), None))
        self._test('comp', glsl.Version('310 es'),
                   (glsl.Version('310 es'), None))
