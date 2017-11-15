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

"""Test the opengl module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
try:
    from unittest import mock
except ImportError:
    import mock

import pytest

from framework import wflinfo
from framework.test import opengl
from framework.test.base import TestIsSkip as _TestIsSkip

from .. import utils

# pylint: disable=no-self-use,attribute-defined-outside-init,protected-access


class TestFastSkipMixin(object):  # pylint: disable=too-many-public-methods
    """Tests for the FastSkipMixin class."""

    @pytest.yield_fixture(autouse=True, scope='class')
    def patch(self):
        """Create a Class with FastSkipMixin, but patch various bits."""
        _mock_wflinfo = mock.Mock(spec=wflinfo.WflInfo)
        _mock_wflinfo.gl_version = 3.3
        _mock_wflinfo.gles_version = 3.0
        _mock_wflinfo.glsl_version = 3.3
        _mock_wflinfo.glsl_es_version = 2.0
        _mock_wflinfo.gl_extensions = set(['bar'])

        with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
            yield

    class _Test(opengl.FastSkipMixin, utils.Test):
        pass

    def test_api(self):
        """Tests that the api works.

        Since you're not suppoed to be able to pass gl and gles version, this
        uses two seperate constructor calls.
        """
        self._Test(['foo'], gl_required={'foo'}, gl_version=3, glsl_version=2)
        self._Test(['foo'], gl_required={'foo'}, gles_version=3,
                   glsl_es_version=2)


class TestFastSkip(object):
    """Tests for the FastSkip class."""

    @pytest.yield_fixture(autouse=True, scope='class')
    def patch(self):
        """Create a Class with FastSkipMixin, but patch various bits."""
        _mock_wflinfo = mock.Mock(spec=wflinfo.WflInfo)
        _mock_wflinfo.gl_version = 3.3
        _mock_wflinfo.gles_version = 3.0
        _mock_wflinfo.glsl_version = 3.3
        _mock_wflinfo.glsl_es_version = 2.0
        _mock_wflinfo.gl_extensions = set(['bar'])

        with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
            yield

    @pytest.fixture
    def inst(self):
        return opengl.FastSkip()

    def test_should_skip(self, inst):
        """test.opengl.FastSkipMixin.test: Skips when requires is missing
        from extensions.
        """
        inst.gl_required.add('foobar')
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_should_not_skip(self, inst):
        """test.opengl.FastSkipMixin.test: runs when requires is in
        extensions.
        """
        inst.gl_required.add('bar')
        inst.test()

    def test_max_gl_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if gl_version >
        __max_gl_version.
        """
        inst.gl_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_gl_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if gl_version <
        __max_gl_version.
        """
        inst.gl_version = 1.0

    def test_max_gl_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if gl_version is None"""
        inst.test()

    def test_max_gles_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if gles_version >
        __max_gles_version.
        """
        inst.gles_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_gles_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if gles_version <
        __max_gles_version.
        """
        inst.gles_version = 1.0

    def test_max_gles_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if gles_version is None"""
        inst.test()

    def test_max_glsl_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if glsl_version >
        __max_glsl_version.
        """
        inst.glsl_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_glsl_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if glsl_version <
        __max_glsl_version.
        """
        inst.glsl_version = 1.0

    def test_max_glsl_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if glsl_version is None"""
        inst.test()

    def test_max_glsl_es_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if glsl_es_version >
        __max_glsl_es_version.
        """
        inst.glsl_es_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_glsl_es_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if glsl_es_version <
        __max_glsl_es_version.
        """
        inst.glsl_es_version = 1.0

    def test_max_glsl_es_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if glsl_es_version is None"""
        inst.test()

    class TestEmpty(object):
        """Tests for the FastSkip class when values are unset."""

        @pytest.yield_fixture(autouse=True, scope='class')
        def patch(self):
            """Create a Class with FastSkipMixin, but patch various bits."""
            _mock_wflinfo = mock.Mock(spec=wflinfo.WflInfo)
            _mock_wflinfo.gl_version = None
            _mock_wflinfo.gles_version = None
            _mock_wflinfo.glsl_version = None
            _mock_wflinfo.glsl_es_version = None
            _mock_wflinfo.gl_extensions = set()

            with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
                yield

        @pytest.fixture
        def inst(self):
            return opengl.FastSkip()

        def test_extension_empty(self, inst):
            """test.opengl.FastSkipMixin.test: if extensions are empty test
            runs.
            """
            inst.gl_required.add('foobar')
            inst.test()

        def test_requires_empty(self, inst):
            """test.opengl.FastSkipMixin.test: if gl_requires is empty test
            runs.
            """
            inst.test()

        def test_max_gl_version_unset(self, inst):
            """test.opengl.FastSkipMixin.test: runs if __max_gl_version is
            None.
            """
            inst.gl_version = 1.0
            inst.test()

        def test_max_glsl_es_version_unset(self, inst):
            """test.opengl.FastSkipMixin.test: runs if __max_glsl_es_version is
            None.
            """
            inst.glsl_es_version = 1.0
            inst.test()

    def test_max_glsl_version_unset(self, inst):
        """test.opengl.FastSkipMixin.test: runs if __max_glsl_version is
        None.
        """
        inst.glsl_version = 1.0
        inst.test()

    def test_max_gles_version_unset(self, inst):
        """test.opengl.FastSkipMixin.test: runs if __max_gles_version is
        None.
        """
        inst.gles_version = 1.0
        inst.test()


class TestFastSkipMixinDisabled(object):
    """Tests for the sub version."""

    @pytest.yield_fixture(autouse=True, scope='class')
    def patch(self):
        """Create a Class with FastSkipMixin, but patch various bits."""
        _mock_wflinfo = mock.Mock(spec=wflinfo.WflInfo)
        _mock_wflinfo.gl_version = 3.3
        _mock_wflinfo.gles_version = 3.0
        _mock_wflinfo.glsl_version = 3.3
        _mock_wflinfo.glsl_es_version = 2.0
        _mock_wflinfo.gl_extensions = set(['bar'])

        with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
            yield

    class _Test(opengl.FastSkipMixin, utils.Test):
        pass

    def test_api(self):
        """Tests that the api works.

        Since you're not suppoed to be able to pass gl and gles version, this
        uses two seperate constructor calls.
        """
        self._Test(['foo'], gl_required={'foo'}, gl_version=3, glsl_version=2)
        self._Test(['foo'], gl_required={'foo'}, gles_version=3,
                   glsl_es_version=2)
