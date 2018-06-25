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

"""Test the wflinfo module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import subprocess
import textwrap
try:
    from unittest import mock
except ImportError:
    import mock

import pytest

from framework import wflinfo

# pylint: disable=no-self-use,attribute-defined-outside-init,protected-access


def _has_wflinfo():
    """Return True if wflinfo is available in PATH."""
    try:
        subprocess.check_call(['wflinfo', '--help'])
    except subprocess.CalledProcessError:
        return False
    except OSError as e:
        if e.errno != 2:
            raise
        return False
    return True


@pytest.mark.skipif(not _has_wflinfo(), reason="Tests require wflinfo binary.")
class TestWflInfo(object):
    """Tests for the WflInfo class."""

    class TestAttributes(object):
        """test for attribute assignments."""

        @pytest.yield_fixture(autouse=True)
        def patch(self):
            """Mock a few things for testing purposes."""
            # This is pretty ugly, but as a Borb with a private shared state,
            # the only way to test this module is to actually replace the
            # shared_state with a mock value so it's reset after each test
            with mock.patch.dict('framework.wflinfo.OPTIONS.env',
                                 {'PIGLIT_PLATFORM': 'foo'}), \
                    mock.patch(
                        'framework.wflinfo.WflInfo._WflInfo__shared_state',
                        {}):
                yield

        def setup(self):
            """Setup each instance, patching necissary bits."""
            self._test = wflinfo.WflInfo()

        def test_extension(self):
            """wflinfo.WflInfo.<api>.extensions: Provides list of gl
            extensions.
            """
            rv = textwrap.dedent("""\
                foo
                bar
                boink
                OpenGL version string: 1.1
                OpenGL shading language: 1.1
                OpenGL extensions: GL_foobar GL_ham_sandwhich
            """).encode('utf-8')
            expected = set(['GL_foobar', 'GL_ham_sandwhich'])

            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.compat.extensions == expected

        def test_api_version(self):
            """wflinfo.WflInfo.<api>.api_version: Provides a version number."""
            rv = textwrap.dedent("""\
                Waffle platform: gbm
                Waffle api: gl
                OpenGL vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL context flags: 0x0
                OpenGL shading language: 1.1
                OpenGL extensions: GL_foobar GL_ham_sandwhich
                OpenGL version string: 18 (Compat Profile) Mesa 11.0.4
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.compat.api_version == 18.0

        def test_api_version_gles(self):
            """wflinfo.WflInfo.<es>.api_version: Provides a version number.

            The format for GLES is different than for Desktop GL.
            """
            rv = textwrap.dedent("""\
                Waffle platform: gbm
                Waffle api: gles3
                OpenGL vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL version string: OpenGL ES 7.1 Mesa 11.0.4
                OpenGL shading language: 1.1
                OpenGL extensions: GL_foobar GL_ham_sandwhich
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.es2.api_version == 7.1

        def test_glsl_version(self):
            """wflinfo.WflInfo.<api>.shader_version: Provides a version number."""
            rv = textwrap.dedent("""\
                Waffle platform: gbm
                Waffle api: gl
                OpenGL vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL version string: 1.1 (Core Profile) Mesa 11.0.4
                OpenGL context flags: 0x0
                OpenGL shading language version string: 9.30
                OpenGL extensions: this is some extension strings.
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.core.shader_version == 9.3

        def test_glsl_es_version(self):
            """wflinfo.WflInfo.<es>.shader_version: works with gles2."""
            rv = textwrap.dedent("""\
                Waffle platform: gbm
                Waffle api: gles2
                Wflinfo vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL version string: OpenGL ES 3.0 Mesa 11.0.4
                OpenGL shading language version string: OpenGL ES GLSL ES 1.0.17
                OpenGL version string: 1.1 (Core Profile) Mesa 11.0.4
                OpenGL extensions: this is some extension strings.
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.es2.shader_version == 1.0

        def test_gl_version_patch(self):
            """wflinfo.WflInfo.*.api_version: Works with patch versions."""
            rv = textwrap.dedent("""\
                Waffle platform: gbm
                Waffle api: gl
                OpenGL vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL version string: 18.0.1 (Core Profile) Mesa 11.0.4
                OpenGL shading language version string: OpenGL ES GLSL ES 5.00
                OpenGL extensions: this is some extension strings.
                OpenGL context flags: 0x0
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.core.api_version == 18.0

        def test_glsl_version_patch(self):
            """wflinfo.WflInfo.*.shader_version: Works with patch versions."""
            rv = textwrap.dedent("""\
                Waffle platform: gbm
                Waffle api: gl
                OpenGL vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL version string: 1.1 (Core Profile) Mesa 11.0.4
                OpenGL context flags: 0x0
                OpenGL shading language version string: 9.30.7
                OpenGL extensions: this is some extension strings.
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.core.shader_version == 9.3

        def test_leading_junk(self):
            """wflinfo.WflInfo.*.api_version: Handles leading junk."""
            rv = textwrap.dedent("""\
                Warning: I'm a big fat warnngs
                Waffle platform: gbm
                Waffle api: gl
                OpenGL vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL version string: 18.0.1 (Core Profile) Mesa 11.0.4
                OpenGL context flags: 0x0
                OpenGL shading language version string: 9.30.7
                OpenGL extensions: ARB_ham_sandwich
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.core.api_version == 18.0

        def test_mixed_junk(self):
            """wflinfo.WflInfo.*.api_version: Handles mixed junk."""
            rv = textwrap.dedent("""\
                Waffle platform: gbm
                Waffle api: gl
                Warning: I'm a big fat warnngs
                OpenGL vendor string: Intel Open Source Technology Center
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                Warning: I'm a big fat warnngs
                Warning: I'm a big fat warnngs
                OpenGL version string: 18.0.1 (Core Profile) Mesa 11.0.4
                OpenGL context flags: 0x0
                OpenGL shading language version string: 9.30.7
                OpenGL extensions: ARB_ham_sandwich
            """).encode('utf-8')
            with mock.patch('framework.wflinfo.subprocess.check_output',
                            mock.Mock(return_value=rv)):
                assert self._test.core.api_version == 18.0

    class TestWAFFLEINFO_GL_ERROR(object):
        """Test class for WflInfo when "WFLINFO_GL_ERROR" is returned."""

        @pytest.yield_fixture(autouse=True)
        def patch(self):
            """Setup each instance, patching necissary bits."""
            rv = textwrap.dedent("""\
                Waffle platform: glx
                Waffle api: gles3
                OpenGL vendor string: WFLINFO_GL_ERROR
                OpenGL renderer string: Mesa DRI Intel(R) Haswell Mobile
                OpenGL version string: WFLINFO_GL_ERROR
                OpenGL context flags: 0x0\n
                OpenGL shading language version string: WFLINFO_GL_ERROR
                OpenGL extensions: WFLINFO_GL_ERROR
            """).encode('utf-8')

            with mock.patch.dict('framework.wflinfo.OPTIONS.env',
                                 {'PIGLIT_PLATFORM': 'foo'}), \
                    mock.patch(
                        'framework.wflinfo.subprocess.check_output',
                        mock.Mock(return_value=rv)):
                yield

        @pytest.fixture(scope='class')
        def inst(self):
            return wflinfo.WflInfo()

        def test_gl_version(self, inst):
            """wflinfo.WflInfo.gl_version: handles WFLINFO_GL_ERROR
            correctly.
            """
            assert inst.core.api_version == 0.0

        def test_glsl_version(self, inst):
            """wflinfo.WflInfo.glsl_version: handles WFLINFO_GL_ERROR
            correctly.
            """
            assert inst.core.shader_version == 0.0

        def test_gl_extensions(self, inst):
            """wflinfo.WflInfo.gl_extensions: handles WFLINFO_GL_ERROR
            correctly.
            """
            assert inst.core.extensions == set()

    class TestOSError(object):
        """Tests for the Wflinfo functions to handle OSErrors."""

        # pylint: disable=pointless-statement

        @pytest.yield_fixture(autouse=True, scope='class')
        def patch(self):
            """Setup the class, patching as necessary."""
            # pylint: disable=bad-continuation
            with mock.patch.dict(
                    'framework.wflinfo.OPTIONS.env',
                    {'PIGLIT_PLATFORM': 'foo'}), \
                    mock.patch(
                        'framework.wflinfo.subprocess.check_output',
                        mock.Mock(side_effect=OSError(2, 'foo'))), \
                    mock.patch(
                        'framework.wflinfo.WflInfo._WflInfo__shared_state',
                        {}):
                yield

        @pytest.fixture
        def inst(self):
            return wflinfo.WflInfo()

        def test_gl_extensions(self, inst):
            """wflinfo.WflInfo.gl_extensions: Handles OSError "no file"
            gracefully.
            """
            assert inst.core.extensions == set()

        def test_gl_version(self, inst):
            """wflinfo.WflInfo.get_gl_version: Handles OSError "no file"
            gracefull.
            """
            assert inst.core.api_version == 0.0

        def test_glsl_version(self, inst):
            """wflinfo.WflInfo.glsl_version: Handles OSError "no file"
            gracefully.
            """
            assert inst.core.shader_version == 0.0

    class TestCalledProcessError(object):
        """Tests for the WflInfo functions to handle OSErrors."""

        # pylint: disable=pointless-statement

        @pytest.yield_fixture(autouse=True, scope='class')
        def patch(self):
            """Setup the class, patching as necessary."""
            # pylint: disable=bad-continuation
            with mock.patch.dict(
                    'framework.wflinfo.OPTIONS.env',
                    {'PIGLIT_PLATFORM': 'foo'}), \
                    mock.patch(
                        # pylint: disable=line-too-long
                        'framework.wflinfo.subprocess.check_output',
                        mock.Mock(side_effect=subprocess.CalledProcessError(1, 'foo'))), \
                    mock.patch(
                        'framework.wflinfo.WflInfo._WflInfo__shared_state',
                        {}):
                yield

        @pytest.fixture
        def inst(self):
            return wflinfo.WflInfo()

        def test_gl_extensions(self, inst):
            """wflinfo.WflInfo.gl_extensions: Handles CalledProcessError
            gracefully.
            """
            assert inst.core.extensions == set()

        def test_gl_version(self, inst):
            """wflinfo.WflInfo.get_gl_version: Handles CalledProcessError
            gracefully.
            """
            assert inst.core.api_version == 0.0

        def test_glsl_version(self, inst):
            """wflinfo.WflInfo.glsl_version: Handles CalledProcessError
            gracefully.
            """
            assert inst.core.shader_version == 0.0
