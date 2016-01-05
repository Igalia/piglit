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
import textwrap

try:
    from unittest import mock
except ImportError:
    import mock

import nose.tools as nt

from . import utils
from framework.test import opengl
from framework.test.base import TestIsSkip

# pylint: disable=invalid-name,protected-access,line-too-long,
# pylint: disable=pointless-statement,attribute-defined-outside-init
# pylint: disable=too-many-public-methods


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

    def test_gl_version(self):
        """test.opengl.WflInfo.gl_version: Provides a version number"""
        rv = (
            'Waffle platform: gbm\n'
            'Waffle api: gl\n'
            'OpenGL vendor string: Intel Open Source Technology Center\n'
            'OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile\n'
            'OpenGL version string: 18 (Core Profile) Mesa 11.0.4\n'
            'OpenGL context flags: 0x0\n'
        )
        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(18.0, self._test.gl_version)

    def test_gles_version(self):
        """test.opengl.WflInfo.gles_version: Provides a version number"""
        rv = (
            'Waffle platform: gbm\n'
            'Waffle api: gles3\n'
            'OpenGL vendor string: Intel Open Source Technology Center\n'
            'OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile\n'
            'OpenGL version string: OpenGL ES 7.1 Mesa 11.0.4\n'
        )
        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(7.1, self._test.gles_version)

    def test_glsl_version(self):
        """test.opengl.WflInfo.glsl_version: Provides a version number"""
        rv = (
            'Waffle platform: gbm\n'
            'Waffle api: gl\n'
            'OpenGL vendor string: Intel Open Source Technology Center\n'
            'OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile\n'
            'OpenGL version string: 1.1 (Core Profile) Mesa 11.0.4\n'
            'OpenGL context flags: 0x0\n'
            'OpenGL shading language version string: 9.30\n'
            'OpenGL extensions: this is some extension strings.\n'
        )
        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(9.3, self._test.glsl_version)

    def test_glsl_es_version_1(self):
        """test.opengl.WflInfo.glsl_es_version: works with gles2"""
        rv = (
            'Waffle platform: gbm\n'
            'Waffle api: gles2\n'
            'OpenGL vendor string: Intel Open Source Technology Center\n'
            'OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile\n'
            'OpenGL version string: OpenGL ES 3.0 Mesa 11.0.4\n'
            'OpenGL shading language version string: OpenGL ES GLSL ES 1.0.17\n'
            'OpenGL extensions: this is some extension strings.\n'
        )
        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(1.0, self._test.glsl_es_version)

    def test_glsl_es_version_3plus(self):
        """test.opengl.WflInfo.glsl_es_version: works with gles3"""
        rv = (
            'Waffle platform: gbm\n'
            'Waffle api: gles3\n'
            'OpenGL vendor string: Intel Open Source Technology Center\n'
            'OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile\n'
            'OpenGL version string: OpenGL ES 3.0 Mesa 11.0.4\n'
            'OpenGL shading language version string: OpenGL ES GLSL ES 5.00\n'
            'OpenGL extensions: this is some extension strings.\n'
        )
        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(5.0, self._test.glsl_es_version)

    def test_gl_version_patch(self):
        """test.opengl.WflInfo.gl_version: Works with patch versions"""
        rv = (
            'Waffle platform: gbm\n'
            'Waffle api: gl\n'
            'OpenGL vendor string: Intel Open Source Technology Center\n'
            'OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile\n'
            'OpenGL version string: 18.0.1 (Core Profile) Mesa 11.0.4\n'
            'OpenGL context flags: 0x0\n'
        )
        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(18.0, self._test.gl_version)

    def test_glsl_version_patch(self):
        """test.opengl.WflInfo.glsl_version: Works with patch versions"""
        rv = (
            'Waffle platform: gbm\n'
            'Waffle api: gl\n'
            'OpenGL vendor string: Intel Open Source Technology Center\n'
            'OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile\n'
            'OpenGL version string: 1.1 (Core Profile) Mesa 11.0.4\n'
            'OpenGL context flags: 0x0\n'
            'OpenGL shading language version string: 9.30.7\n'
            'OpenGL extensions: this is some extension strings.\n'
        )
        with mock.patch('framework.test.opengl.subprocess.check_output',
                        mock.Mock(return_value=rv)):
            nt.eq_(9.3, self._test.glsl_version)


class TestWflInfo_WAFFLEINFO_GL_ERROR(object):
    """Test class for WflInfo when "WFLINFO_GL_ERROR" is returned."""
    __patchers = []

    def setup(self):
        """Setup each instance, patching necissary bits."""
        self._test = opengl.WflInfo()
        self.__patchers.append(mock.patch.dict(
            'framework.test.opengl.OPTIONS.env',
            {'PIGLIT_PLATFORM': 'foo'}))
        self.__patchers.append(mock.patch(
            'framework.test.opengl.WflInfo._WflInfo__shared_state', {}))

        rv = (textwrap.dedent("""
            Waffle platform: glx
            Waffle api: gles3
            OpenGL vendor string: WFLINFO_GL_ERROR
            OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
            OpenGL version string: WFLINFO_GL_ERROR
            OpenGL context flags: 0x0\n
            OpenGL shading language version string: WFLINFO_GL_ERROR
            OpenGL extensions: WFLINFO_GL_ERROR
        """))

        self.__patchers.append(mock.patch(
            'framework.test.opengl.subprocess.check_output',
            mock.Mock(return_value=rv)))

        for f in self.__patchers:
            f.start()

    def teardown(self):
        for p in self.__patchers:
            p.stop()

    def test_gl_version(self):
        """test.opengl.WflInfo.gl_version: handles WFLINFO_GL_ERROR correctly"""
        nt.eq_(None, self._test.gl_version)

    def test_gles_version(self):
        """test.opengl.WflInfo.gles_version: handles WFLINFO_GL_ERROR correctly"""
        nt.eq_(None, self._test.gles_version)

    def test_glsl_version(self):
        """test.opengl.WflInfo.glsl_version: handles WFLINFO_GL_ERROR correctly"""
        nt.eq_(None, self._test.glsl_version)

    def test_glsl_es_version(self):
        """test.opengl.WflInfo.glsl_es_version: handles WFLINFO_GL_ERROR correctly"""
        nt.eq_(None, self._test.glsl_es_version)

    def test_gl_extensions(self):
        """test.opengl.WflInfo.gl_extensions: handles WFLINFO_GL_ERROR correctly"""
        nt.eq_(set(), self._test.gl_extensions)


class TestWflInfoOSError(object):
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

    @utils.not_raises(OSError)
    def test_gl_version(self):
        """test.opengl.WflInfo.get_gl_version: Handles OSError "no file" gracefully"""
        self.inst.gl_version

    @utils.not_raises(OSError)
    def test_gles_version(self):
        """test.opengl.WflInfo.get_gles_version: Handles OSError "no file" gracefully"""
        self.inst.gles_version

    @utils.not_raises(OSError)
    def test_glsl_version(self):
        """test.opengl.WflInfo.glsl_version: Handles OSError "no file" gracefully"""
        self.inst.glsl_version

    @utils.not_raises(OSError)
    def test_glsl_es_version(self):
        """test.opengl.WflInfo.glsl_es_version: Handles OSError "no file" gracefully"""
        self.inst.glsl_es_version


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

    @utils.not_raises(subprocess.CalledProcessError)
    def test_gl_version(self):
        """test.opengl.WflInfo.get_gl_version: Handles CalledProcessError gracefully"""
        self.inst.gl_version

    @utils.not_raises(subprocess.CalledProcessError)
    def test_gles_version(self):
        """test.opengl.WflInfo.gles_version: Handles CalledProcessError gracefully"""
        self.inst.gles_version

    @utils.not_raises(subprocess.CalledProcessError)
    def test_glsl_version(self):
        """test.opengl.WflInfo.glsl_version: Handles CalledProcessError gracefully"""
        self.inst.glsl_version

    @utils.not_raises(subprocess.CalledProcessError)
    def test_glsl_es_version(self):
        """test.opengl.WflInfo.glsl_es_version: Handles CalledProcessError gracefully"""
        self.inst.glsl_es_version


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

    @nt.raises(TestIsSkip)
    def test_max_gl_version_lt(self):
        """tefst.opengl.FastSkipMixin.is_skip: skips if gl_version > __max_gl_version"""
        self.test.gl_version = 4.0
        self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_gl_version_gt(self):
        """test.opengl.FastSkipMixin.is_skip: runs if gl_version < __max_gl_version"""
        self.test.gl_version = 1.0

    @utils.not_raises(TestIsSkip)
    def test_max_gl_version_unset(self):
        """test.opengl.FastSkipMixin.is_skip: runs if __max_gl_version is None"""
        self.test.gl_version = 1.0
        with mock.patch.object(self.test._FastSkipMixin__info, 'gl_version',  # pylint: disable=no-member
                               None):
            self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_gl_version_set(self):
        """test.opengl.FastSkipMixin.is_skip: runs if gl_version is None"""
        self.test.is_skip()

    @nt.raises(TestIsSkip)
    def test_max_gles_version_lt(self):
        """test.opengl.FastSkipMixin.is_skip: skips if gles_version > __max_gles_version"""
        self.test.gles_version = 4.0
        self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_gles_version_gt(self):
        """test.opengl.FastSkipMixin.is_skip: runs if gles_version < __max_gles_version"""
        self.test.gles_version = 1.0

    @utils.not_raises(TestIsSkip)
    def test_max_gles_version_unset(self):
        """test.opengl.FastSkipMixin.is_skip: runs if __max_gles_version is None"""
        self.test.gles_version = 1.0
        with mock.patch.object(self.test._FastSkipMixin__info, 'gles_version',  # pylint: disable=no-member
                               None):
            self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_gles_version_set(self):
        """test.opengl.FastSkipMixin.is_skip: runs if gles_version is None"""
        self.test.is_skip()

    @nt.raises(TestIsSkip)
    def test_max_glsl_version_lt(self):
        """test.opengl.FastSkipMixin.is_skip: skips if glsl_version > __max_glsl_version"""
        self.test.glsl_version = 4.0
        self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_glsl_version_gt(self):
        """test.opengl.FastSkipMixin.is_skip: runs if glsl_version < __max_glsl_version"""
        self.test.glsl_version = 1.0

    @utils.not_raises(TestIsSkip)
    def test_max_glsl_version_unset(self):
        """test.opengl.FastSkipMixin.is_skip: runs if __max_glsl_version is None"""
        self.test.glsl_version = 1.0
        with mock.patch.object(self.test._FastSkipMixin__info, 'glsl_version',  # pylint: disable=no-member
                               None):
            self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_glsl_version_set(self):
        """test.opengl.FastSkipMixin.is_skip: runs if glsl_version is None"""
        self.test.is_skip()

    @nt.raises(TestIsSkip)
    def test_max_glsl_es_version_lt(self):
        """test.opengl.FastSkipMixin.is_skip: skips if glsl_es_version > __max_glsl_es_version"""
        self.test.glsl_es_version = 4.0
        self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_glsl_es_version_gt(self):
        """test.opengl.FastSkipMixin.is_skip: runs if glsl_es_version < __max_glsl_es_version"""
        self.test.glsl_es_version = 1.0

    @utils.not_raises(TestIsSkip)
    def test_max_glsl_es_version_unset(self):
        """test.opengl.FastSkipMixin.is_skip: runs if __max_glsl_es_version is None"""
        self.test.glsl_es_version = 1.0
        with mock.patch.object(self.test._FastSkipMixin__info, 'glsl_es_version',  # pylint: disable=no-member
                               None):
            self.test.is_skip()

    @utils.not_raises(TestIsSkip)
    def test_max_glsl_es_version_set(self):
        """test.opengl.FastSkipMixin.is_skip: runs if glsl_es_version is None"""
        self.test.is_skip()
