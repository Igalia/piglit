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

import os
import nose.tools as nt
import framework.glsl_parser_test as glsl
import framework.tests.utils as utils
from framework.core import testBinDir


def _check_config(content):
    """ This is the test that actually checks the glsl config section """
    with utils.with_tempfile(content) as tfile:
        return glsl.GLSLParserTest(tfile), tfile


def test_glslparser_initializer():
    """ Test that GLSLParserTest initializes """
    glsl.GLSLParserTest('spec/glsl-es-1.00/execution/sanity.shader_test')


def test_cpp_comments():
    """ Test C++ style comments """
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// [end config]\n')
    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(testBinDir, 'glslparsertest'),
                                   name, 'pass', '1.00'],
                    msg="C++ style comments were not properly parsed")


def test_c_comments():
    """ Test C style comments """
    content = ('/*\n'
               ' * [config]\n'
               ' * expect_result: pass\n'
               ' * glsl_version: 1.00\n'
               ' * [end config]\n'
               ' */\n')
    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(testBinDir, 'glslparsertest'),
                                   name, 'pass', '1.00'],
                    msg="C style comments were not properly parsed")


@nt.raises(Exception)
def test_no_config_end():
    """ end_config section is required """
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '//\n')
    _, _ = _check_config(content)


@nt.raises(Exception)
def test_no_expecte_result():
    """ expect_result section is required """
    content = ('// [config]\n'
               '// glsl_version: 1.00\n'
               '//\n')
    _, _ = _check_config(content)


@nt.raises(Exception)
def test_no_required_glsl_version():
    """ glsl_version section is required """
    content = ('//\n'
               '// expect_result: pass\n'
               '// [end config]\n')
    _, _ = _check_config(content)


@nt.raises(Exception)
def test_no_config_start():
    """ Config section is required """
    content = ('//\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// [end config]\n')
    _, _ = _check_config(content)


def test_blank_in_config():
    """ C++ style comments can have uncommented newlines """
    content = ('// [config]\n'
               '\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// [end config]\n')

    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(testBinDir, 'glslparsertest'),
                                   name, 'pass', '1.00'],
                    msg="A newline in a C++ style comment was not properly "
                        "parsed.")
