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

"""Test the opengl module."""

from __future__ import absolute_import, division, print_function
import subprocess

import mock
import nose.tools as nt

import framework.tests.utils as utils
from framework.test import opengl
from framework.test.base import TestIsSkip

# pylint: disable=invalid-name,protected-access,line-too-long,pointless-statement,attribute-defined-outside-init


class TestWflInfo(object):
    """Tests for the WflInfo class."""
    __patchers = []

    def setup(self):
        """Setup each instance, patching necissary bits."""
        self._test = opengl.WflInfo()
        self.__patchers.append(mock.patch.dict(
            'framework.test.opengl.OPTIONS.env',
            {'PIGLIT_PLATFORM': 'foo'}))
        self.__patchers.append(mock.patch(
            'framework.test.opengl.WflInfo._WflInfo__shared_state', {}))

        for f in self.__patchers:
            f.start()

    def teardown(self):
        for f in self.__patchers:
            f.stop()

    def test_gl_extension(self):
        """test.opengl.WflInfo.gl_extensions: Provides list of gl extensions"""
        rv = 'foo\nbar\nboink\nOpenGL extensions: GL_foobar GL_ham_sandwhich\n'
        expected = set(['GL_foobar', 'GL_ham_sandwhich'])

        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(expected, self._test.gl_extensions)


class TestWflInfoSError(object):
    """Tests for the Wflinfo functions to handle OSErrors."""
    __patchers = []

    @classmethod
    def setup_class(cls):
        """Setup the class, patching as necissary."""
        cls.__patchers.append(mock.patch.dict(
            'framework.test.opengl.OPTIONS.env',
            {'PIGLIT_PLATFORM': 'foo'}))
        cls.__patchers.append(mock.patch(
            'framework.test.opengl.subprocess.check_output',
            mock.Mock(side_effect=OSError(2, 'foo'))))
        cls.__patchers.append(mock.patch(
            'framework.test.opengl.WflInfo._WflInfo__shared_state', {}))

        for f in cls.__patchers:
            f.start()

    def setup(self):
        self.inst = opengl.WflInfo()

    @classmethod
    def teardown_class(cls):
        for f in cls.__patchers:
            f.stop()

    @utils.not_raises(OSError)
    def test_gl_extensions(self):
        """test.opengl.WflInfo.gl_extensions: Handles OSError "no file" gracefully"""
        self.inst.gl_extensions


class TestWflInfoCalledProcessError(object):
    """Tests for the WflInfo functions to handle OSErrors."""
    __patchers = []

    @classmethod
    def setup_class(cls):
        """Setup the class, patching as necissary."""
        cls.__patchers.append(mock.patch.dict(
            'framework.test.opengl.OPTIONS.env',
            {'PIGLIT_PLATFORM': 'foo'}))
        cls.__patchers.append(mock.patch(
            'framework.test.opengl.subprocess.check_output',
            mock.Mock(side_effect=subprocess.CalledProcessError(1, 'foo'))))
        cls.__patchers.append(mock.patch(
            'framework.test.opengl.WflInfo._WflInfo__shared_state', {}))

        for f in cls.__patchers:
            f.start()

    @classmethod
    def teardown_class(cls):
        for f in cls.__patchers:
            f.stop()

    def setup(self):
        self.inst = opengl.WflInfo()

    @utils.not_raises(subprocess.CalledProcessError)
    def test_gl_extensions(self):
        """test.opengl.WflInfo.gl_extensions: Handles CalledProcessError gracefully"""
        self.inst.gl_extensions


class TestFastSkipMixin(object):
    """Tests for the FastSkipMixin class."""
    __patchers = []

    @classmethod
    def setup_class(cls):
        """Create a Class with FastSkipMixin, but patch various bits."""
        class _Test(opengl.FastSkipMixin, utils.Test):
            pass

        cls._class = _Test

        _mock_wflinfo = mock.Mock(spec=opengl.WflInfo)
        _mock_wflinfo.gl_version = 3.3
        _mock_wflinfo.gles_version = 3.0
        _mock_wflinfo.glsl_version = 3.3
        _mock_wflinfo.glsl_es_version = 2.0
        _mock_wflinfo.gl_extensions = set(['bar'])

        cls.__patchers.append(mock.patch.object(
            _Test, '_FastSkipMixin__info', _mock_wflinfo))

        for patcher in cls.__patchers:
            patcher.start()

    @classmethod
    def teardown_class(cls):
        for patcher in cls.__patchers:
            patcher.stop()

    def setup(self):
        self.test = self._class(['foo'])

    @nt.raises(TestIsSkip)
    def test_should_skip(self):
        """test.opengl.FastSkipMixin.is_skip: Skips when requires is missing from extensions"""
        self.test.gl_required.add('foobar')
        self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_should_not_skip(self):
        """test.opengl.FastSkipMixin.is_skip: runs when requires is in extensions"""
        self.test.gl_required.add('bar')
        self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_extension_empty(self):
        """test.opengl.FastSkipMixin.is_skip: if extensions are empty test runs"""
        self.test.gl_required.add('foobar')
        with mock.patch.object(self.test._FastSkipMixin__info, 'gl_extensions',  # pylint: disable=no-member
                               None):
            self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_requires_empty(self):
        """test.opengl.FastSkipMixin.is_skip: if gl_requires is empty test runs"""
        self.test.is_skip()
