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
        _mock_wflinfo.core.api_version = 3.3
        _mock_wflinfo.core.shader_version = 3.3
        _mock_wflinfo.core.extensions = set(['bar'])
        _mock_wflinfo.es2.api_version = 3.0
        _mock_wflinfo.es2.shader_version = 2.0
        _mock_wflinfo.es2.extensions = set(['bar'])

        with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
            yield

    class _Test(opengl.FastSkipMixin, utils.Test):
        pass

    def test_api(self):
        """Tests that the api works.

        Since you're not suppoed to be able to pass gl and gles version, this
        uses two seperate constructor calls.
        """
        self._Test(['foo'], extensions={'foo'}, api_version=3, shader_version=2)
        self._Test(['foo'], extensions={'foo'}, api_version=3,
                   shader_version=2)


class TestFastSkip(object):
    """Tests for the FastSkip class."""

    @pytest.yield_fixture(autouse=True, scope='class')
    def patch(self):
        """Create a Class with FastSkipMixin, but patch various bits."""
        _mock_wflinfo = mock.Mock(spec=wflinfo.WflInfo)
        _mock_wflinfo.core.api_version = 3.3
        _mock_wflinfo.core.shader_version = 3.3
        _mock_wflinfo.core.extensions = set(['bar'])
        _mock_wflinfo.es2.api_version = 3.0
        _mock_wflinfo.es2.shader_version = 2.0
        _mock_wflinfo.es2.extensions = set(['bar'])

        with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
            yield

    @pytest.fixture
    def inst(self):
        return opengl.FastSkip(api='core')

    def test_should_skip(self, inst):
        """test.opengl.FastSkipMixin.test: Skips when requires is missing
        from extensions.
        """
        inst.extensions.add('foobar')
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_should_not_skip(self, inst):
        """test.opengl.FastSkipMixin.test: runs when requires is in
        extensions.
        """
        inst.extensions.add('bar')
        inst.test()

    def test_max_api_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if api_version >
        __max_api_version.
        """
        inst.api_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_api_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if api_version <
        __max_api_version.
        """
        inst.api_version = 1.0

    def test_max_api_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if api_version is None"""
        inst.test()

    def test_max_api_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if api_version >
        __max_api_version.
        """
        inst.api_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_api_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if api_version <
        __max_api_version.
        """
        inst.api_version = 1.0

    def test_max_api_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if api_version is None"""
        inst.test()

    def test_max_shader_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if shader_version >
        __max_shader_version.
        """
        inst.shader_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_shader_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if shader_version <
        __max_shader_version.
        """
        inst.shader_version = 1.0

    def test_max_shader_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if shader_version is None"""
        inst.test()

    def test_max_shader_version_lt(self, inst):
        """test.opengl.FastSkipMixin.test: skips if shader_version >
        __max_shader_version.
        """
        inst.shader_version = 4.0
        with pytest.raises(_TestIsSkip):
            inst.test()

    def test_max_shader_version_gt(self, inst):
        """test.opengl.FastSkipMixin.test: runs if shader_version <
        __max_shader_version.
        """
        inst.shader_version = 1.0

    def test_max_shader_version_set(self, inst):
        """test.opengl.FastSkipMixin.test: runs if shader_version is None"""
        inst.test()

    class TestEmpty(object):
        """Tests for the FastSkip class when values are unset."""

        @pytest.yield_fixture(autouse=True, scope='class')
        def patch(self):
            """Create a Class with FastSkipMixin, but patch various bits."""
            _mock_wflinfo = mock.Mock(spec=wflinfo.WflInfo)
            _mock_wflinfo.core.api_version = 0.0
            _mock_wflinfo.core.shader_version = 0.0
            _mock_wflinfo.core.extensions = set()
            _mock_wflinfo.es2.api_version = 0.0
            _mock_wflinfo.es2.shader_version = 0.0
            _mock_wflinfo.es2.extensions = set()

            with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
                yield

        @pytest.fixture
        def inst(self):
            return opengl.FastSkip(api='core')

        def test_extension_empty(self, inst):
            """test.opengl.FastSkipMixin.test: if extensions are empty test
            runs.
            """
            inst.info.core.extensions.add('foobar')
            inst.test()

        def test_requires_empty(self, inst):
            """test.opengl.FastSkipMixin.test: if gl_requires is empty test
            runs.
            """
            inst.test()

    def test_max_shader_version_unset(self, inst):
        """test.opengl.FastSkipMixin.test: runs if __max_shader_version is
        None.
        """
        inst.shader_version = 1.0
        inst.test()

    def test_max_api_version_unset(self, inst):
        """test.opengl.FastSkipMixin.test: runs if __max_api_version is
        None.
        """
        inst.api_version = 1.0
        inst.test()


class TestFastSkipMixinDisabled(object):
    """Tests for the sub version."""

    @pytest.yield_fixture(autouse=True, scope='class')
    def patch(self):
        """Create a Class with FastSkipMixin, but patch various bits."""
        _mock_wflinfo = mock.Mock(spec=wflinfo.WflInfo)
        _mock_wflinfo.es2.api_version = 3.3
        _mock_wflinfo.es2.shader_version = 2.0
        _mock_wflinfo.es2.extensions = set(['bar'])
        _mock_wflinfo.core.api_version = 3.0
        _mock_wflinfo.core.shader_version = 3.3
        _mock_wflinfo.core.extensions = set(['bar'])

        with mock.patch('framework.test.opengl.FastSkip.info', _mock_wflinfo):
            yield

    class _Test(opengl.FastSkipMixin, utils.Test):
        pass

    def test_api(self):
        """Tests that the api works.

        Since you're not suppoed to be able to pass gl and gles version, this
        uses two seperate constructor calls.
        """
        self._Test(['foo'], extensions={'foo'}, api_version=3, shader_version=2)
        self._Test(['foo'], extensions={'foo'}, api_version=3,
                   shader_version=2)
