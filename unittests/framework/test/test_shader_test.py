# Copyright (c) 2014, 2016 Intel Corporation

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

""" Provides tests for the shader_test module """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import textwrap
try:
    import mock
except ImportError:
    from unittest import mock

import pytest
import six

from framework import status
from framework.test import shader_test

# pylint: disable=invalid-name,no-self-use,protected-access


class _Setup(object):
    def __init__(self):
        self.__patchers = []
        self.__patchers.append(mock.patch.dict(
            'framework.test.base.OPTIONS.env',
            {'PIGLIT_PLATFORM': 'foo'}))

    def setup(self, _):
        for patcher in self.__patchers:
            patcher.start()

    def teardown(self, _):
        for patcher in self.__patchers:
            patcher.stop()


_setup = _Setup()
setup_module = _setup.setup
teardown_module = _setup.teardown


class TestConfigParsing(object):
    """Tests for ShaderRunner config parsing."""

    @pytest.mark.parametrize('gles,operator,expected', [
        # pylint: disable=bad-whitespace
        ('2.0', '>=', 'shader_runner_gles2'),
        ('2.0', '<=', 'shader_runner_gles2'),
        ('2.0', '=',  'shader_runner_gles2'),
        ('2.0', '>',  'shader_runner_gles3'),
        ('3.0', '>=', 'shader_runner_gles3'),
        ('3.0', '<=', 'shader_runner_gles2'),
        ('3.0', '=',  'shader_runner_gles3'),
        ('3.0', '>',  'shader_runner_gles3'),
        ('3.0', '<',  'shader_runner_gles2'),
        ('3.1', '>=', 'shader_runner_gles3'),
        ('3.1', '<=', 'shader_runner_gles2'),
        ('3.1', '=',  'shader_runner_gles3'),
        ('3.1', '>',  'shader_runner_gles3'),
        ('3.1', '<',  'shader_runner_gles2'),
        ('3.2', '>=', 'shader_runner_gles3'),
        ('3.2', '<=', 'shader_runner_gles2'),
        ('3.2', '=',  'shader_runner_gles3'),
        ('3.2', '<',  'shader_runner_gles2'),
    ])
    def test_bin(self, gles, operator, expected, tmpdir):
        """Test that the shader_runner parse picks the correct binary."""
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL ES {} {}
            GLSL ES >= 1.00

            [next section]
            """.format(operator, gles)))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert os.path.basename(test.command[0]) == expected

    def test_gles3_bin(self, tmpdir):
        """test.shader_test.ShaderTest: Identifies GLES3 tests successfully."""
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL ES >= 3.0
            GLSL ES >= 3.00 es
            """))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert os.path.basename(test.command[0]) == "shader_runner_gles3"

    def test_skip_gl_required(self, tmpdir):
        """test.shader_test.ShaderTest: populates gl_requirements properly"""
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL >= 3.0
            GL_ARB_ham_sandwhich
            """))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert test.require_extensions == {'GL_ARB_ham_sandwhich'}

    def test_skip_gl_version(self, tmpdir):
        """test.shader_test.ShaderTest: finds gl_version."""
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL >= 2.0
            GL_ARB_ham_sandwhich
            """))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert test.require_version == 2.0

    def test_skip_gles_version(self, tmpdir):
        """test.shader_test.ShaderTest: finds gles_version."""
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL ES >= 2.0
            GL_ARB_ham_sandwhich
            """))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert test.require_version == 2.0

    def test_skip_glsl_version(self, tmpdir):
        """test.shader_test.ShaderTest: finds glsl_version."""
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL >= 2.1
            GLSL >= 1.20
            """))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert test.require_shader == 1.2

    def test_skip_glsl_es_version(self, tmpdir):
        """test.shader_test.ShaderTest: finds glsl_es_version."""
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL ES >= 2.0
            GLSL ES >= 1.00
            """))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert test.require_shader == 1.0

    def test_ignore_directives(self, tmpdir):
        """There are some directives for shader_runner that are not interpreted
        by the python layer, they are only for the C layer. These should be
        ignored by the python layer.
        """
        p = tmpdir.join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL >= 3.3
            GLSL >= 1.50
            GL_MAX_VERTEX_OUTPUT_COMPONENTS
            GL_MAX_FRAGMENT_UNIFORM_COMPONENTS
            GL_MAX_VERTEX_UNIFORM_COMPONENTS
            GL_MAX_VARYING_COMPONENTS
            GL_ARB_foobar
            """))
        test = shader_test.ShaderTest.new(six.text_type(p))

        assert test.require_version == 3.3
        assert test.require_shader == 1.50
        assert test.require_extensions == {'GL_ARB_foobar'}


class TestCommand(object):
    """Tests for the command property."""

    @pytest.fixture(scope='class')
    def test_file(self, tmpdir_factory):
        p = tmpdir_factory.mktemp('shader-test-command').join('test.shader_test')
        p.write(textwrap.dedent("""\
            [require]
            GL ES >= 3.0
            GLSL ES >= 3.00 es
            """))
        return six.text_type(p)

    def test_getter_adds_auto_and_fbo(self, test_file):
        """test.shader_test.ShaderTest: -auto and -fbo is added to the command.
        """
        test = shader_test.ShaderTest.new(test_file)
        assert '-auto' in test.command
        assert '-fbo' in test.command

    def test_setter_doesnt_add_auto_and_fbo(self, test_file):
        """Don't add -fbo or -auto to self._command when using the setter."""
        test = shader_test.ShaderTest.new(test_file)
        test.command += ['-newarg']
        assert '-auto' not in test._command
        assert '-fbo' not in test._command


class TestMultiShaderTest(object):
    """Tests for the MultiShaderTest class."""

    class TestConstructor(object):
        """Tests for the constructor object."""

        @pytest.fixture
        def inst(self, tmpdir):
            """A fixture that creates an instance to test."""
            one = tmpdir.join('foo.shader_test')
            one.write(textwrap.dedent("""\
                [require]
                GLSL >= 3.0

                [vertex shader]"""))
            two = tmpdir.join('bar.shader_test')
            two.write(textwrap.dedent("""\
                [require]
                GLSL >= 4.0

                [vertex shader]"""))

            return shader_test.MultiShaderTest.new(
                [six.text_type(one), six.text_type(two)])

        def test_prog(self, inst):
            assert os.path.basename(inst.command[0]) == 'shader_runner'

        def test_filenames(self, inst):
            assert os.path.basename(inst.command[1]) == 'foo.shader_test'
            assert os.path.basename(inst.command[2]) == 'bar.shader_test'

        def test_extra(self, inst):
            assert inst.command[3] == '-auto'

    @pytest.fixture
    def inst(self, tmpdir):
        """A fixture that creates an instance to test."""
        one = tmpdir.join('foo.shader_test')
        one.write(textwrap.dedent("""\
            [require]
            GLSL >= 3.0

            [vertex shader]"""))
        two = tmpdir.join('bar.shader_test')
        two.write(textwrap.dedent("""\
            [require]
            GLSL >= 4.0
            GL_ARB_ham_sandwhich

            [vertex shader]"""))

        return shader_test.MultiShaderTest.new(
            [six.text_type(one), six.text_type(two)])

    def test_resume(self, inst):
        actual = inst._resume(1)  # pylint: disable=protected-access
        assert os.path.basename(actual[0]) == 'shader_runner'
        assert os.path.basename(actual[1]) == 'bar.shader_test'
        assert os.path.basename(actual[2]) == '-auto'

    def test_skips_set(self, inst):
        assert inst.skips[0].shader_version == 3.0
        assert inst.skips[1].shader_version == 4.0
        assert inst.skips[1].extensions == {'GL_ARB_ham_sandwhich'}

    def test_process_skips(self, inst):
        expected = {'bar': status.SKIP, 'foo': status.NOTRUN}
        with mock.patch.object(inst.skips[0].info.core, 'shader_version', 3.0):
            inst._process_skips()
        assert dict(inst.result.subtests) == expected
