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
from framework.exectest import TEST_BIN_DIR


def _check_config(content):
    """ This is the test that actually checks the glsl config section """
    with utils.with_tempfile(content) as tfile:
        return glsl.GLSLParserTest(tfile), tfile


def test_no_config_start():
    """ GLSLParserTest requires [config] """
    content = ('// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// [end config]\n')
    with utils.with_tempfile(content) as tfile:
        with nt.assert_raises(glsl.GLSLParserException) as exc:
            glsl.GLSLParserTest(tfile)
            nt.assert_equal(
                exc.exception, 'No [config] section found!',
                msg="No config section found, no exception raised")


def test_find_config_start():
    """ GLSLParserTest finds [config] """
    content = ('// [config]\n'
               '// glsl_version: 1.00\n'
               '//\n')
    with utils.with_tempfile(content) as tfile:
        with nt.assert_raises(glsl.GLSLParserException) as exc:
            glsl.GLSLParserTest(tfile)
            nt.assert_not_equal(
                exc.exception, 'No [config] section found!',
                msg="Config section not parsed")


def test_no_config_end():
    """ GLSLParserTest requires [end config] """
    with utils.with_tempfile('// [config]\n') as tfile:
        with nt.assert_raises(glsl.GLSLParserException) as exc:
            glsl.GLSLParserTest(tfile)
            nt.assert_equal(
                exc.exception, 'No [end config] section found!',
                msg="config section not closed, no exception raised")


def test_no_expect_result():
    """ expect_result section is required """
    content = ('// [config]\n'
               '// glsl_version: 1.00\n'
               '//\n')
    with utils.with_tempfile(content) as tfile:
        with nt.assert_raises(glsl.GLSLParserException) as exc:
            glsl.GLSLParserTest(tfile)
            nt.assert_equal(
                exc.exception,
                'Missing required section expect_result from config',
                msg="config section not closed, no exception raised")


def test_no_glsl_version():
    """ glsl_version section is required """
    content = ('//\n'
               '// expect_result: pass\n'
               '// [end config]\n')
    with utils.with_tempfile(content) as tfile:
        with nt.assert_raises(glsl.GLSLParserException) as exc:
            glsl.GLSLParserTest(tfile)
            nt.assert_equal(
                exc.exception,
                'Missing required section glsl_version from config',
                msg="config section not closed, no exception raised")


def test_cpp_comments():
    """ Parses C++ style comments """
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// [end config]\n')
    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(TEST_BIN_DIR, 'glslparsertest'),
                                   name, 'pass', '1.00'],
                    msg="C++ style comments were not properly parsed")


def test_c_comments():
    """ Parses C style comments """
    content = ('/*\n'
               ' * [config]\n'
               ' * expect_result: pass\n'
               ' * glsl_version: 1.00\n'
               ' * [end config]\n'
               ' */\n')
    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(TEST_BIN_DIR, 'glslparsertest'),
                                   name, 'pass', '1.00'],
                    msg="C style comments were not properly parsed")


def test_blank_in_config():
    """ C++ style comments can have uncommented newlines """
    content = ('// [config]\n'
               '\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// [end config]\n')

    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(TEST_BIN_DIR, 'glslparsertest'),
                                   name, 'pass', '1.00'],
                    msg="A newline in a C++ style comment was not properly "
                        "parsed.")


def test_empty_in_config():
    """ C++ sytle comments can have blank commented lines """
    content = ('// [config]\n'
               '//\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// [end config]\n')

    test, name = _check_config(content)

    nt.assert_equal(test.command, [os.path.join(TEST_BIN_DIR, 'glslparsertest'),
                                   name, 'pass', '1.00'],
                    msg="A blank commented line in a C++ style comment was not"
                        " properly parsed.")


def test_glslparser_initializer():
    """ GLSLParserTest initializes """
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
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.00\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.00'],
         'all required options'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.00\n//check_link: true\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.00', '--check-link'],
         'check_link true'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.00\n//check_link: false\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.00'],
         'check_link false'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.00\n//require_extensions: ARB_foo\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.00', 'ARB_foo'],
         'one required_extension'),
        ('// [config]\n// expect_result: pass\n// glsl_version: 1.00\n//require_extensions: ARB_foo ARB_bar\n// [end config]\n',
         [os.path.join(TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.00', 'ARB_foo', 'ARB_bar'],
         'multiple required_extensions'),
    ]

    for config, result, desc in content:
        check_config_to_command.description = \
            'Command correctly generated for {}'.format(desc)
        yield check_config_to_command, config, result


def test_bad_section_name():
    """ A section name not in the _CONFIG_KEYS name raises an error """
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// new_awesome_key: foo\n'
               '// [end config]\n')

    with nt.assert_raises(glsl.GLSLParserException) as e:
        _, name = _check_config(content)

        nt.eq_(e.exception.message,
               'Key new_awesome_key in file {0 is not a valid key for a '
               'glslparser test config block'.format(name))


def test_good_section_names():
    """ A section name in the _CONFIG_KEYS does not raise an error """
    content = ('// [config]\n'
               '// expect_result: pass\n'
               '// glsl_version: 1.00\n'
               '// require_extensions: EXT_foo\n'
               '// check_link: True\n'
               '// [end config]\n')

    try:
        _check_config(content)
    except glsl.GLSLParserException as e:
        raise AssertionError(e)


def check_no_duplicates(content, dup):
    """ Ensure that duplicate entries raise an error """
    with nt.assert_raises(glsl.GLSLParserException) as e:
        with utils.with_tempfile(content) as tfile:
            glsl.GLSLParserTest(tfile)

            nt.eq_(
                e.exception.message,
                'Duplicate entry for key {0} in file {1}'.format(dup, tfile))


@utils.nose_generator
def test_duplicate_entries():
    """ Generate tests for duplicate keys in the config block """
    content = [
        ('expect_result', '// expect_result: pass\n'),
        ('glsl_version', '// glsl_version: 1.00\n'),
        ('require_extensions', '// require_extensions: ARB_ham_sandwhich\n')
    ]

    for name, value in content:
        check_no_duplicates.description = \
            "duplicate values of {0} raise an exception".format(name)
        test = '// [config]\n{0}{1}// [end config]'.format(
            ''.join(x[1] for x in content), value)

        yield check_no_duplicates, test, name
