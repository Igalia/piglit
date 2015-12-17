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

""" Provides tests for the shader_test module """

from __future__ import print_function, absolute_import
import os

import mock
import nose.tools as nt

from framework import exceptions
import framework.test as testm
from . import utils

# pylint: disable=invalid-name


class _Setup(object):
    def __init__(self):
        self.__patchers = []
        self.__patchers.append(mock.patch.dict(
            'framework.test.base.options.OPTIONS.env',
            {'PIGLIT_PLATFORM': 'foo'}))

    def setup(self):
        for patcher in self.__patchers:
            patcher.start()

    def teardown(self):
        for patcher in self.__patchers:
            patcher.stop()


_setup = _Setup()
setup = _setup.setup
teardown = _setup.teardown


def test_initialize_shader_test():
    """test.shader_test.ShaderTest: class initializes"""
    testm.ShaderTest('tests/spec/glsl-es-1.00/execution/sanity.shader_test')


def test_parse_gl_test_no_decimal():
    """test.shader_test.ShaderTest: raises if version lacks decminal"""
    data = ('[require]\n'
            'GL = 2\n')
    with utils.tempfile(data) as temp:
        with nt.assert_raises(exceptions.PiglitFatalError) as exc:
            testm.ShaderTest(temp)
            nt.assert_equal(exc.exception, "No GL version set",
                            msg="A GL version was passed without a decimal, "
                                "which should have raised an exception, but "
                                "did not")


def test_parse_gles2_test():
    """test.shader_test.ShaderTest: Identifies GLES2 tests successfully"""
    data = ('[require]\n'
            'GL ES >= 2.0\n'
            'GLSL ES >= 1.00\n')
    with utils.tempfile(data) as temp:
        test = testm.ShaderTest(temp)

    nt.assert_equal(
        os.path.basename(test.command[0]), "shader_runner_gles2",
        msg="This test should have run with shader_runner_gles2, "
            "but instead ran with " + os.path.basename(test.command[0]))


def test_parse_gles3_test():
    """test.shader_test.ShaderTest: Identifies GLES3 tests successfully"""
    data = ('[require]\n'
            'GL ES >= 3.0\n'
            'GLSL ES >= 3.00\n')
    with utils.tempfile(data) as temp:
        test = testm.ShaderTest(temp)

    nt.assert_equal(
        os.path.basename(test.command[0]), "shader_runner_gles3",
        msg="This test should have run with shader_runner_gles3, "
            "but instead ran with " + os.path.basename(test.command[0]))


def test_add_auto():
    """test.shader_test.ShaderTest: -auto is added to the command"""
    test = testm.ShaderTest('tests/spec/glsl-es-1.00/execution/sanity.shader_test')
    nt.assert_in('-auto', test.command)


def test_find_requirements_gl_requirements():
    """test.shader_test.ShaderTest: populates gl_requirements properly"""

    data = ('[require]\n'
            'GL = 2.0\n'
            'GL_ARB_ham_sandwhich\n')

    with utils.tempfile(data) as temp:
        test = testm.ShaderTest(temp)

    nt.eq_(test.gl_required, set(['GL_ARB_ham_sandwhich']))


def test_find_requirements_gl_version():
    """test.shader_test.ShaderTest: finds gl_version."""
    data = ('[require]\n'
            'GL = 2.0\n'
            'GL_ARB_ham_sandwhich\n')

    with mock.patch('framework.test.shader_test.open',
                    mock.mock_open(read_data=data)):
        test = testm.ShaderTest('null')
    nt.eq_(test.gl_version, 2.0)


def test_find_requirements_gles_version():
    """test.shader_test.ShaderTest: finds gles_version."""
    data = ('[require]\n'
            'GL ES = 2.0\n'
            'GL_ARB_ham_sandwhich\n')

    with mock.patch('framework.test.shader_test.open',
                    mock.mock_open(read_data=data)):
        test = testm.ShaderTest('null')
    nt.eq_(test.gles_version, 2.0)


def test_find_requirements_glsl_version():
    """test.shader_test.ShaderTest: finds glsl_version."""
    data = ('[require]\n'
            'GL = 2.0\n'
            'GLSL >= 1.0\n'
            'GL_ARB_ham_sandwhich\n')

    with mock.patch('framework.test.shader_test.open',
                    mock.mock_open(read_data=data)):
        test = testm.ShaderTest('null')
    nt.eq_(test.glsl_version, 1.0)


def test_find_requirements_glsl_es_version():
    """test.shader_test.ShaderTest: finds glsl_es_version."""
    data = ('[require]\n'
            'GL ES = 2.0\n'
            'GLSL ES > 2.00\n'
            'GL_ARB_ham_sandwhich\n')

    with mock.patch('framework.test.shader_test.open',
                    mock.mock_open(read_data=data)):
        test = testm.ShaderTest('null')
    nt.eq_(test.glsl_es_version, 2.0)


@utils.nose_generator
def test_ignore_shader_runner_directives():
    """test.shader_test.ShaderTest: Doesn't add shader_runner command to gl_required list"""
    should_ignore = [
        'GL_MAX_VERTEX_OUTPUT_COMPONENTS',
        'GL_MAX_FRAGMENT_UNIFORM_COMPONENTS',
        'GL_MAX_VERTEX_UNIFORM_COMPONENTS',
        'GL_MAX_VARYING_COMPONENTS',
    ]

    def test(config):
        with mock.patch('framework.test.shader_test.open',
                        mock.mock_open(read_data=config)):
            test = testm.ShaderTest('null')
        nt.eq_(test.gl_required, {'GL_foobar'})

    for ignore in should_ignore:
        config = '\n'.join([
            '[require]',
            'GL >= 1.0',
            'GL_foobar',
            ignore,
        ])
        test.description = ('test.shader_test.ShaderTest: doesn\'t add '
                            'shader_runner command {} to gl_required'.format(
                                ignore))

        yield test, config
