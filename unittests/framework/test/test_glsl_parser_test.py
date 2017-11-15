# Copyright (c) 2014-2016 Intel Corporation

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

"""Tests for framework.test.glsl_parser_test."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools
import os
import textwrap
# pylint: disable=import-error
try:
    from unittest import mock
except ImportError:
    import mock
# pylint: enable=import-error

import pytest
import six

from framework import exceptions
from framework.test import glsl_parser_test as glsl
from framework.test.piglit_test import TEST_BIN_DIR as _TEST_BIN_DIR
from framework.test.base import TestIsSkip as _TestIsSkip

# pylint: disable=invalid-name


class _Setup(object):
    """A class holding setup and teardown methods.

    These methods need to share data, and a class is a nice way to encapsulate
    that.

    """
    def __init__(self):
        self.patchers = [
            mock.patch('framework.test.glsl_parser_test._HAS_GL_BIN', True),
            mock.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', True),
            mock.patch.dict('framework.wflinfo.OPTIONS.env',
                            {'PIGLIT_PLATFORM': 'foo'}),
        ]

    def setup(self, _):
        for p in self.patchers:
            p.start()

    def teardown(self, _):
        for p in self.patchers:
            p.stop()


_setup = _Setup()
setup_module = _setup.setup
teardown_module = _setup.teardown


def test_no_config_start(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: exception is raised if [config]
    section is missing."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""
        // expect_result: pass
        // glsl_version: 1.10
        // [end config]"""))

    with pytest.raises(glsl.GLSLParserNoConfigError):
        glsl.GLSLParserTest(six.text_type(p))


def test_find_config_start(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: successfully finds [config]
    section."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""
        // [config]
        // expect_result: pass
        // glsl_version: 1.10"""))

    with pytest.raises(exceptions.PiglitFatalError):
        glsl.GLSLParserTest(six.text_type(p))


def test_no_config_end(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: exception is raised if [end
    config] section is missing."""
    p = tmpdir.join('test.frag')
    p.write('// [config]')

    with pytest.raises(exceptions.PiglitFatalError):
        glsl.GLSLParserTest(six.text_type(p))


def test_no_expect_result(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: exception is raised if
    "expect_result" key is missing."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        // [config]
        // glsl_version: 1.10
        // [end config]"""))

    with pytest.raises(exceptions.PiglitFatalError):
        glsl.GLSLParserTest(six.text_type(p))


def test_no_glsl_version(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: exception is raised if
    "glsl_version" key is missing."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        // [config]
        // expect_result: pass
        // [end config]"""))

    with pytest.raises(exceptions.PiglitFatalError):
        glsl.GLSLParserTest(six.text_type(p))


def test_cpp_comments(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: parses C++ style comments ('//').
    """
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        // [config]
        // expect_result: pass
        // glsl_version: 1.10
        // [end config]"""))
    test = glsl.GLSLParserTest(six.text_type(p))

    assert test.command == [os.path.join(_TEST_BIN_DIR, 'glslparsertest'),
                            six.text_type(p), 'pass', '1.10']


def test_c_comments(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: parses C++ style comments ('/* */')
    """
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        /* [config]
         * expect_result: pass
         * glsl_version: 1.10
         * [end config]
         */"""))

    test = glsl.GLSLParserTest(six.text_type(p))

    assert test.command == [os.path.join(_TEST_BIN_DIR, 'glslparsertest'),
                            six.text_type(p), 'pass', '1.10']


def test_blank_in_config_cpp(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: C++ style comments can have
    uncommented newlines."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        // [config]

        // expect_result: pass
        // glsl_version: 1.10
        // [end config]"""))
    test = glsl.GLSLParserTest(six.text_type(p))

    assert test.command == [os.path.join(_TEST_BIN_DIR, 'glslparsertest'),
                            six.text_type(p), 'pass', '1.10']


def test_empty_in_config_cpp(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: C++ style comments can have blank
    commented lines."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        // [config]
        //
        // expect_result: pass
        // glsl_version: 1.10
        // [end config]"""))
    test = glsl.GLSLParserTest(six.text_type(p))

    assert test.command == [os.path.join(_TEST_BIN_DIR, 'glslparsertest'),
                            six.text_type(p), 'pass', '1.10']


def test_blank_in_config_c(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: C style comments can have
    uncommented newlines."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        /* [config]

         * expect_result: pass
         * glsl_version: 1.10
         * [end config]
         */"""))
    test = glsl.GLSLParserTest(six.text_type(p))

    assert test.command == [os.path.join(_TEST_BIN_DIR, 'glslparsertest'),
                            six.text_type(p), 'pass', '1.10']


def test_empty_in_config_c(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: C style comments can have blank
    commented lines."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        /* [config]
         *
         * expect_result: pass
         * glsl_version: 1.10
         * [end config]
         */"""))
    test = glsl.GLSLParserTest(six.text_type(p))

    assert test.command == [os.path.join(_TEST_BIN_DIR, 'glslparsertest'),
                            six.text_type(p), 'pass', '1.10']


@pytest.mark.parametrize(
    "config,expected",
    # pylint: disable=line-too-long
    [('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n// [end config]\n',
      [os.path.join(_TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10']),
     ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//check_link: true\n// [end config]\n',
      [os.path.join(_TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10', '--check-link']),
     ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//check_link: false\n// [end config]\n',
      [os.path.join(_TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10']),
     ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//require_extensions: ARB_foo\n// [end config]\n',
      [os.path.join(_TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10', 'ARB_foo']),
     ('// [config]\n// expect_result: pass\n// glsl_version: 1.10\n//require_extensions: ARB_foo ARB_bar\n// [end config]\n',
      [os.path.join(_TEST_BIN_DIR, 'glslparsertest'), 'pass', '1.10', 'ARB_foo', 'ARB_bar'])],
    # pylint: enable=line-too-long
    ids=['all required options', 'check_link true', 'check_link false',
         'one required_extension', 'multiple required_exetension'])
def test_config_to_command(config, expected, tmpdir):
    """Test that config blocks are converted into the expected commands."""
    p = tmpdir.join('test.frag')
    p.write(config)
    test = glsl.GLSLParserTest(six.text_type(p))
    # add the filename, which isn't known util now
    expected.insert(1, six.text_type(p))

    assert test.command == expected


def test_bad_section_name(tmpdir):
    """test.glsl_parser_test.GLSLParserTest: Unknown config keys cause an
    error."""
    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        // [config]
        // expect_result: pass
        // glsl_version: 1.10
        // new_awesome_key: foo
        // [end config]"""))

    with pytest.raises(exceptions.PiglitFatalError):
        glsl.GLSLParserTest(six.text_type(p))


@pytest.mark.parametrize(
    "extra",
    ['expect_result: pass', 'glsl_version: 1.10',
     'require_extensions: ARB_ham_sandwhich', 'check_link: false'],
    ids=['expect_result', 'glsl_version', 'require_extensions', 'check_link'])
def test_duplicate_entry(extra, tmpdir):
    """Test that duplicate entries are an error."""
    p = tmpdir.join('test.vert')
    p.write(textwrap.dedent("""\
        // [config]
        // expect_result: pass
        // glsl_version: 1.10
        // require_extensions: ARB_foobar
        // check_link: True
        // {}
        // [end config]""".format(extra)))

    with pytest.raises(exceptions.PiglitFatalError):
        glsl.GLSLParserTest(six.text_type(p))


@pytest.mark.parametrize(
    "separator",
    ['ARB_ham, ARB_turkey', 'ARB_pork; ARB_chicken', 'ARB_foo;'],
    ids=['comma separated', 'semicolon separated', 'trailing semicolon'])
def test_invalid_extensions_separator(separator, tmpdir):
    """Test that invalid extension separators are rejected."""
    p = tmpdir.join('test.vert')
    p.write(textwrap.dedent("""\
        // [config]
        // expect_result: pass
        // glsl_version: 1.10
        // require_extensions: ARB_foobar
        // check_link: True
        // require_extensions: {}
        // [end config]""".format(separator)))

    with pytest.raises(exceptions.PiglitFatalError):
        glsl.GLSLParserTest(six.text_type(p))


@pytest.mark.parametrize(
    "ext",
    ['GL_EXT_foo', '!GL_EXT_foo', 'GL_EXT_foo GL_ARB_foo',
     '!GL_EXT_foo !GL_ARB_foo', '!GL_EXT_foo GL_ARB_foo'],
    ids=['single require', 'single exclude', 'multiple require',
         'multiple exclude', 'mixed require and exclude'])
def test_valid_extensions(ext, tmpdir):
    """Test that invalid extension separators are rejected."""
    p = tmpdir.join('test.vert')
    p.write(textwrap.dedent("""\
        // [config]
        // expect_result: pass
        // glsl_version: 1.10
        // require_extensions: {}
        // [end config]""".format(ext)))

    expected = ext.split(' ')
    test = glsl.GLSLParserTest(six.text_type(p))

    assert test.command[-len(expected):] == expected


@pytest.mark.parametrize(
    "version,has_bin,forced",
    itertools.product(
        ['1.00', '3.00', '3.10', '3.20', '3.00 es', '3.10 es', '3.20 es'],
        [True, False], [True, False]))
def test_get_glslparsertest_gles2(version, has_bin, forced, tmpdir, mocker):
    """Tests for assigning the correct binary for GLES tests.

    Tests with and without the gles binary and with and without the force
    desktop mode.
    """
    if not has_bin or forced:
        expected = 'glslparsertest'
    else:
        expected = 'glslparsertest_gles2'

    mocker.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', has_bin)
    mocker.patch('framework.test.glsl_parser_test._FORCE_DESKTOP_VERSION',
                 forced)

    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        /* [config]
         * expect_result: pass
         * glsl_version: {}
         * [end config]
         */""".format(version)))
    inst = glsl.GLSLParserTest(six.text_type(p))

    assert os.path.basename(inst.command[0]) == expected


class TestGLSLParserTestSkipRequirements(object):
    """Tests for setting FastSkip parameters."""
    @staticmethod
    def write_config(filename, version='4.3', extra=''):
        filename.write(textwrap.dedent("""\
            // [config]
            // expect_result: pass
            // glsl_version: {}
            // {}
            // [end config]""".format(version, extra)))

    def test_glsl_version(self, tmpdir):
        p = tmpdir.join('test.frag')
        self.write_config(p)
        assert glsl.GLSLParserTest(six.text_type(p)).glsl_version == 4.3

    def test_glsl_es_version(self, tmpdir):
        p = tmpdir.join('test.frag')
        self.write_config(p, version='3.0')
        assert glsl.GLSLParserTest(six.text_type(p)).glsl_es_version == 3.0

    def test_gl_required(self, tmpdir):
        p = tmpdir.join('test.frag')
        self.write_config(p, extra="require_extensions: GL_ARB_foo GL_ARB_bar")
        assert glsl.GLSLParserTest(six.text_type(p)).gl_required == \
            {'GL_ARB_foo', 'GL_ARB_bar'}

    def test_exclude_not_added_to_gl_required(self, tmpdir):
        p = tmpdir.join('test.frag')
        self.write_config(p, extra="require_extensions: GL_ARB_foo !GL_ARB_bar")
        assert glsl.GLSLParserTest(six.text_type(p)).gl_required == \
            {'GL_ARB_foo'}


def test_skip_desktop_without_binary(tmpdir, mocker):
    """There is no way to run desktop tests with only GLES compiled make sure
    we don't try.
    """
    mocker.patch('framework.test.glsl_parser_test._HAS_GL_BIN', False)

    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        /* [config]
         * expect_result: pass
         * glsl_version: 1.10
         * [end config]
         */"""))
    test = glsl.GLSLParserTest(six.text_type(p))

    with pytest.raises(_TestIsSkip):
        test.is_skip()


@pytest.mark.parametrize("version,extension", [
    ('1.00', 'ARB_ES2_compatibility'),
    ('3.00', 'ARB_ES3_compatibility'),
    ('3.10', 'ARB_ES3_1_compatibility'),
    ('3.20', 'ARB_ES3_2_compatibility'),
])
def test_add_compatibility_requirement_fastskip(version, extension, tmpdir,
                                                mocker):
    """When running GLES tests using the GL binary ensure that the proper
    ARB_ES<ver> compatibility extension is added to the requirements.

    This test checks the fast skipping variable
    """
    mocker.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', False)

    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        /* [config]
         * expect_result: pass
         * glsl_version: {}
         * require_extensions: GL_ARB_ham_sandwhich
         * [end config]
         */""".format(version)))
    test = glsl.GLSLParserTest(six.text_type(p))

    # The arb_compat extension was added to the fast skipping arguments
    assert extension in test.gl_required



@pytest.mark.parametrize("version,extension", [
    ('1.00', 'ARB_ES2_compatibility'),
    ('3.00', 'ARB_ES3_compatibility'),
    ('3.10', 'ARB_ES3_1_compatibility'),
    ('3.20', 'ARB_ES3_2_compatibility'),
])
def test_add_compatibility_requirement_binary(version, extension, tmpdir,
                                              mocker):
    """When running GLES tests using the GL binary ensure that the proper
    ARB_ES<ver> compatibility extension is added to the requirements.

    This test checks the glslparsertest binary command line.
    """
    mocker.patch('framework.test.glsl_parser_test._HAS_GLES_BIN', False)

    p = tmpdir.join('test.frag')
    p.write(textwrap.dedent("""\
        /* [config]
         * expect_result: pass
         * glsl_version: {}
         * require_extensions: GL_ARB_ham_sandwhich
         * [end config]
         */""".format(version)))
    test = glsl.GLSLParserTest(six.text_type(p))

    # The compat extension was added to the slow skipping (C level)
    # requirements
    assert extension in test.command
