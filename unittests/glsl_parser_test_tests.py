# Copyright (c) 2014, 2015 Intel Corporation

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
    from unittest import mock
except ImportError:
    import mock

import nose.tools as nt

from . import utils
import framework.test.glsl_parser_test as glsl
from framework import exceptions
from framework.test import TEST_BIN_DIR, TestIsSkip

# pylint: disable=line-too-long,invalid-name


class _Setup(object):
    """A class holding setup and teardown methods.

    These methods need to share data, and a class is a nice way to encapsulate
    that.

    """
    def __init__(self):
        self.patchers = [
            mock.patch('framework.test.glsl_parser_test._HAS_GL_BIN', True),
            mock.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', True),
            mock.patch.dict('framework.test.opengl.OPTIONS.env', {'PIGLIT_PLATFORM': 'foo'}),
        ]

    def setup(self):
        for p in self.patchers:
            p.start()

    def teardown(self):
        for p in self.patchers:
            p.stop()


_setup = _Setup()
setup = _setup.setup
teardown = _setup.teardown


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
                msg="No config section was found and no exception raised")


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
                    msg="C style (/* */) comments were not properly parsed")


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
                    msg="A newline in a C++ style comment (//) was not "
                        "properly parsed.")


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
    content = textwrap.dedent("""\
        /*
         * [config]
         * expect_result: pass
         * glsl_version: 1.10
         * [end config]
         */
        """)

    with utils.tempfile(content) as f:
        glsl.GLSLParserTest(f)


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


@utils.nose_generator
def test_get_glslparsertest_gles2():
    """GLSLParserTest: gets gles2 binary if glsl is 1.00 or 3.00"""
    def test(content, expected):
        with utils.tempfile(content) as f:
            t = glsl.GLSLParserTest(f)
            nt.eq_(os.path.basename(t.command[0]), expected)

    content = textwrap.dedent("""\
        /*
         * [config]
         * expect_result: pass
         * glsl_version: {}
         * [end config]
         */
        """)
    versions = ['1.00', '3.00', '3.10', '3.20', '3.00 es', '3.10 es',
                '3.20 es']
    description = ("test.glsl_parser_test.GLSLParserTest: "
                   "gets gles2 binary if glsl is '{}' and gles2 binary exists")

    for version in versions:
        test.description = description.format(version)
        yield test, content.format(version), 'glslparsertest_gles2'

    description = ("test.glsl_parser_test.GLSLParserTest: "
                   "gets gl binary if glsl is '{}' and gles2 binary doesn't exist")

    with mock.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', False):
        for version in versions:
            test.description = description.format(version)
            yield test, content.format(version), 'glslparsertest'

    description = ("test.glsl_parser_test.GLSLParserTest: "
                   "gets gl binary if glsl is '{}' and "
                   "PIGLIT_FORCE_GLSLPARSER_DESKTOP is true")

    with mock.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', False):
        with mock.patch('framework.test.glsl_parser_test._FORCE_DESKTOP_VERSION', True):
            for version in versions:
                test.description = description.format(version)
                yield test, content.format(version), 'glslparsertest'


def test_set_glsl_version():
    """test.glsl_parser_test.GLSLParserTest: sets glsl_version"""
    rt = {'glsl_version': '4.3'}
    with mock.patch.object(glsl.GLSLParserTest, '_GLSLParserTest__parser',
                           mock.Mock(return_value=rt)):
        with mock.patch.object(glsl.GLSLParserTest,
                               '_GLSLParserTest__get_command',
                               return_value=['foo']):
            with mock.patch('framework.test.glsl_parser_test.open',
                            mock.mock_open(), create=True):
                with mock.patch('framework.test.glsl_parser_test.os.stat',
                                mock.mock_open()):
                    test = glsl.GLSLParserTest('foo')
    nt.eq_(test.glsl_version, 4.3)


def test_set_glsl_es_version():
    """test.glsl_parser_test.GLSLParserTest: sets glsl_es_version"""
    rt = {'glsl_version': '3.00 es'}
    with mock.patch.object(glsl.GLSLParserTest, '_GLSLParserTest__parser',
                           mock.Mock(return_value=rt)):
        with mock.patch.object(glsl.GLSLParserTest,
                               '_GLSLParserTest__get_command',
                               return_value=['foo']):
            with mock.patch('framework.test.glsl_parser_test.open',
                            mock.mock_open(), create=True):
                with mock.patch('framework.test.glsl_parser_test.os.stat',
                                mock.mock_open()):
                    test = glsl.GLSLParserTest('foo')
    nt.eq_(test.glsl_es_version, 3.0)


def test_set_gl_required():
    """test.glsl_parser_test.GLSLParserTest: sets gl_required"""
    rt = {'require_extensions': 'GL_ARB_foobar GL_EXT_foobar'}
    with mock.patch.object(glsl.GLSLParserTest, '_GLSLParserTest__parser',
                           mock.Mock(return_value=rt)):
        with mock.patch.object(glsl.GLSLParserTest,
                               '_GLSLParserTest__get_command',
                               return_value=['foo']):
            with mock.patch('framework.test.glsl_parser_test.open',
                            mock.mock_open(), create=True):
                with mock.patch('framework.test.glsl_parser_test.os.stat',
                                mock.mock_open()):
                    test = glsl.GLSLParserTest('foo')
    nt.eq_(test.gl_required, set(['GL_ARB_foobar', 'GL_EXT_foobar']))


def test_set_exclude_gl_required():
    """test.glsl_parser_test.GLSLParserTest: doesn't add excludes to gl_required"""
    rt = {'require_extensions': 'GL_ARB_foobar !GL_EXT_foobar'}
    with mock.patch.object(glsl.GLSLParserTest, '_GLSLParserTest__parser',
                           mock.Mock(return_value=rt)):
        with mock.patch.object(glsl.GLSLParserTest,
                               '_GLSLParserTest__get_command',
                               return_value=['foo']):
            with mock.patch('framework.test.glsl_parser_test.open',
                            mock.mock_open(), create=True):
                with mock.patch('framework.test.glsl_parser_test.os.stat',
                                mock.mock_open()):
                    test = glsl.GLSLParserTest('foo')
    nt.eq_(test.gl_required, set(['GL_ARB_foobar']))


@mock.patch('framework.test.glsl_parser_test._HAS_GL_BIN', False)
@nt.raises(TestIsSkip)
def test_binary_skip():
    """test.glsl_parser_test.GLSLParserTest.is_skip: skips OpenGL tests when not built with desktop support"""
    content = textwrap.dedent("""\
        /*
         * [config]
         * expect_result: pass
         * glsl_version: 1.10
         * [end config]
         */
        """)

    with utils.tempfile(content) as f:
        test = glsl.GLSLParserTest(f)
        test.is_skip()


@utils.nose_generator
def test_add_compatability():
    """test.glsl_parser_test.GLSLParserTest: Adds ARB_ES<ver>_COMPATIBILITY
    when shader is gles but only gl is available"""
    content = textwrap.dedent("""\
        /*
         * [config]
         * expect_result: pass
         * glsl_version: {}
         * require_extensions: GL_ARB_ham_sandwhich
         * [end config]
         */
        """)

    @mock.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', False)
    def test(ver, expected):
        with utils.tempfile(content.format(ver)) as f:
            test = glsl.GLSLParserTest(f)
        nt.assert_in(expected, test.gl_required)

    desc = ('test.glsl_parser_test.GLSLParserTest: Add {} to gl_extensions '
            'for GLES tests on OpenGL')

    vers = [
        ('1.00', 'ARB_ES2_compatibility'),
        ('3.00', 'ARB_ES3_compatibility'),
        ('3.10', 'ARB_ES3_1_compatibility'),
        ('3.20', 'ARB_ES3_2_compatibility'),
    ]

    for ver, expected in vers:
        test.description = desc.format(expected)
        yield test, ver, expected
