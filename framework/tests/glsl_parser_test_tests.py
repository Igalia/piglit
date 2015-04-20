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

import nose.tools as nt

from framework import exceptions
import framework.test.glsl_parser_test as glsl
import framework.tests.utils as utils
from framework.test import TEST_BIN_DIR

# pylint: disable=line-too-long,invalid-name


def _check_config(content):
    """ This is the test that actually checks the glsl config section """
    with utils.tempfile(content) as tfile:
        return glsl.GLSLParserTest(tfile), tfile


def test_no_config_start():
    """test.glsl_parser_test.GLSLParserTest: exception is raised if [config] section is missing
    """
    content = ('// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '// [end config]\n')
    with utils.tempfile(content) as tfile:
        with nt.assert_raises(glsl.GLSLParserNoConfigError) as exc:
            glsl.GLSLParserTest(tfile)
            nt.assert_equal(
                exc.exception, 'No [config] section found!',
                msg="No config section found, no exception raised")


@nt.raises(exceptions.PiglitFatalError)
def test_find_config_start():
    """test.glsl_parser_test.GLSLParserTest: successfully finds [config] section
    """
    content = ('// [config]\n'
               '// glsl_version: 1.10\n'
               '//\n')
    with utils.tempfile(content) as tfile:
        glsl.GLSLParserTest(tfile)


@nt.raises(exceptions.PiglitFatalError)
def test_no_config_end():
    """test.glsl_parser_test.GLSLParserTest: exception is raised if [end config] section is missing
    """
    with utils.tempfile('// [config]\n') as tfile:
        glsl.GLSLParserTest(tfile)


@nt.raises(exceptions.PiglitFatalError)
def test_no_expect_result():
    """test.glsl_parser_test.GLSLParserTest: exception is raised if "expect_result" key is missing
    """
    content = ('// [config]\n'
               '// glsl_version: 1.10\n'
               '//\n')
    with utils.tempfile(content) as tfile:
        glsl.GLSLParserTest(tfile)


@nt.raises(exceptions.PiglitFatalError)
def test_no_glsl_version():
    """test.glsl_parser_test.GLSLParserTest: exception is raised if "glsl_version" key is missing
    """
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// [end config]\n')
    with utils.tempfile(content) as tfile:
        glsl.GLSLParserTest(tfile)


def test_cpp_comments():
    """test.glsl_parser_test.GLSLParserTest: parses C++ style comments ('//')
    """
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '// [end config]\n')
    test, name = _check_config(content)

    nt.assert_equal(
        test.command,
        [os.path.join(TEST_BIN_DIR, 'glslparsertest'), name, 'pass', '1.10'])


def test_c_comments():
    """test.glsl_parser_test.GLSLParserTest: parses C++ style comments ('/* */')
    """
    content = ('/*\n'
               ' * [config]\n'
               ' * expect_result: pass\n'
               ' * glsl_version: 1.10\n'
               ' * [end config]\n'
               ' */\n')
    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(TEST_BIN_DIR, 'glslparsertest'),
                                   name, 'pass', '1.10'],
                    msg="C style comments were not properly parsed")


def test_blank_in_config():
    """test.glsl_parser_test.GLSLParserTest: C++ style comments can have uncommented newlines
    """
    content = ('// [config]\n'
               '\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '// [end config]\n')

    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(TEST_BIN_DIR, 'glslparsertest'),
                                   name, 'pass', '1.10'],
                    msg="A newline in a C++ style comment was not properly "
                        "parsed.")


def test_empty_in_config():
    """test.glsl_parser_test.GLSLParserTest: C style comments can have blank newlines
    """
    content = ('// [config]\n'
               '//\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '// [end config]\n')

    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(TEST_BIN_DIR, 'glslparsertest'),
                                   name, 'pass', '1.10'],
                    msg="A blank commented line in a C++ style comment was not"
                        " properly parsed.")


def test_glslparser_initializer():
    """test.glsl_parser_test.GLSLParserTest: clas initializes correctly"""
    # TODO: use a tempfile and write a temporary test rather than rely on a
    # real one
    glsl.GLSLParserTest('tests/spec/glsl-es-1.00/compiler/version-macro.frag')


def check_config_to_command(config, result):
    """ Check that the config is correctly converted """
    inst, f = _check_config(config)
    result.insert(1, f)  # Add the file name

    nt.eq_(inst.command, result)


@utils.nose_generator
def test_config_to_command():
    """ Generate tests that confirm the config file is correctly parsed """
    content = [
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10'],
         'all required options'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//check_link: true\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10', '--check-link'],
         'check_link true'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//check_link: false\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10'],
         'check_link false'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//require_extensions: ARB_foo\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10', 'ARB_foo'],
         'one required_extension'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//require_extensions: ARB_foo ARB_bar\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10', 'ARB_foo', 'ARB_bar'],
         'multiple required_extensions'),
    ]

    for config, result, desc in content:
        check_config_to_command.description = (
            'test.glsl_parser_test.GLSLParserTest.command: '
            'correctly generated for {}'.format(desc))
        yield check_config_to_command, config, result


@nt.raises(exceptions.PiglitFatalError)
def test_bad_section_name():
    """test.glsl_parser_test.GLSLParserTest: A section name not in the _CONFIG_KEYS name raises an error"""
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '// new_awesome_key: foo\n'
               '// [end config]\n')

    with utils.tempfile(content) as tfile:
        glsl.GLSLParserTest(tfile)


@utils.not_raises(exceptions.PiglitFatalError)
def test_good_section_names():
    """test.glsl_parser_test.GLSLParserTest: A section name in the _CONFIG_KEYS does not raise an error"""
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '// require_extensions: EXT_foo\n'
               '// check_link: True\n'
               '// [end config]\n')

    _check_config(content)


@utils.nose_generator
def test_duplicate_entries():
    """ Generate tests for duplicate keys in the config block """

    @nt.raises(exceptions.PiglitFatalError)
    def check_no_duplicates(content):
        """ Ensure that duplicate entries raise an error """
        with utils.tempfile(content) as tfile:
            glsl.GLSLParserTest(tfile)


    content = [
        ('expect_result', '// expect_result: pass\n'),
        ('glsl_version', '// glsl_version: 1.10\n'),
        ('require_extensions', '// require_extensions: ARB_ham_sandwhich\n')
    ]

    for name, value in content:
        check_no_duplicates.description = (
            "test.glsl_parser_test.GLSLParserTest: duplicate values of "
            "{0} raise an exception".format(name))
        test = '// [config]\n{0}{1}// [end config]'.format(
            ''.join(x[1] for x in content), value)

        yield check_no_duplicates, test


@utils.nose_generator
def glslparser_exetensions_seperators():
    """ GlslParserTest() can only have [A-Za-z_] as characters

    This test generates a number of tests that should catch the majority of
    errors relating to seperating extensions in the config block of a
    glslparser test

    """
    @nt.raises(exceptions.PiglitFatalError)
    def check_bad_character(tfile):
        """ Check for bad characters """
        glsl.GLSLParserTest(tfile)

    problems = [
        ('comma seperator', '// require_extensions: ARB_ham, ARB_turkey\n'),
        ('semi-colon seperator', '// require_extensions: ARB_ham; ARB_turkey\n'),
        ('trailing semi-colon', '// require_extensions: ARB_ham ARB_turkey\n;'),
        ('Non-alpha character', '// require_extensions: ARB_$$$\n'),
    ]

    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '{}'
               '// [end config]\n')

    for name, value in problems:
        test = content.format(value)
        with utils.tempfile(test) as tfile:
            check_bad_character.description = (
                'test.glsl_parser_test.GLSLParserTest: require_extensions {0} '
                'should raise an error'.format(name))
            yield check_bad_character, tfile


@utils.nose_generator
def test_good_extensions():
    """ Generates tests with good extensions which shouldn't raise errors """

    @utils.not_raises(exceptions.PiglitFatalError)
    def check_good_extension(file_):
        """ A good extension should not raise a GLSLParserException """
        glsl.GLSLParserTest(file_)

    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.10\n'
               '// require_extensions: {}\n'
               '// [end config]\n')
    options = [
        'GL_EXT_texture_array',
        'GL_EXT_texture_array ARB_example',
        '!GL_ARB_ham_sandwhich',
    ]

    for x in options:
        test = content.format(x)
        check_good_extension.description = (
            'test.glsl_parser_test.GLSLParserTest: '
            'require_extension {} is valid'.format(x))

        with utils.tempfile(test) as tfile:
            yield check_good_extension, tfile
