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

""" Module providing tests for the core module """

from __future__ import print_function, absolute_import
import os
import collections
import shutil
import ConfigParser
import textwrap
import functools

import nose.tools as nt

import framework.tests.utils as utils
import framework.core as core

_CONF_FILE = textwrap.dedent("""\
[nose-test]
; a section for testing behavior
dir = foo
""")


def _save_core_config(func):
    """Decorator function that saves and restores environment state.

    This allows the test to modify any protected state, and guarantees that it
    will be restored afterwords.

    Curerntly this protects a <piglit path>/piglit.conf, and $XDG_CONFIG_HOME
    and $HOME

    """
    @functools.wraps(func)
    def inner(*args, **kwargs):
        """Save and restore state."""
        saved_env = {}
        restore_piglitconf = False

        try:
            for env in ['XDG_CONFIG_HOME', 'HOME']:
                if env in os.environ:
                    saved_env[env] = os.environ.pop(env)

            if os.path.exists('piglit.conf'):
                shutil.move('piglit.conf', 'piglit.conf.restore')
                restore_piglitconf = True
            core.PIGLIT_CONFIG = ConfigParser.SafeConfigParser(
                allow_no_value=True)
        except Exception as e:
            raise utils.UtilsError(e)

        func(*args, **kwargs)

        try:
            for env in ['XDG_CONFIG_HOME', 'HOME']:
                if env in saved_env:
                    os.environ[env] = saved_env[env]
                elif env in os.environ:
                    del os.environ[env]

            if restore_piglitconf:
                shutil.move('piglit.conf.restore', 'piglit.conf')
            core.PIGLIT_CONFIG = ConfigParser.SafeConfigParser(
                allow_no_value=True)
        except Exception as e:
            raise utils.UtilsError(e)

    return inner


def _reset_piglit_config():
    """ Set core.PIGLIT_CONFIG back to pristine """
    core.PIGLIT_CONFIG = ConfigParser.SafeConfigParser()


@utils.no_error
def test_options_init():
    """core.Options(): Class initializes"""
    core.Options()


def test_parse_listfile_return():
    """ Test that parse_listfile returns a container

    Given a file with a newline seperated list of results, parse_listfile
    should return a list of files with no whitespace

    """
    contents = "/tmp/foo\n/tmp/bar\n"

    with utils.with_tempfile(contents) as tfile:
        results = core.parse_listfile(tfile)

    assert isinstance(results, collections.Container)


class Test_parse_listfile_TrailingWhitespace(object):
    """Test that parse_listfile removes whitespace"""
    @classmethod
    def setup_class(cls):
        contents = "/tmp/foo\n/tmp/foo  \n/tmp/foo\t\n"
        with utils.with_tempfile(contents) as tfile:
            cls.results = core.parse_listfile(tfile)

    def test_newlines(self):
        """core.parse_listfile: Remove trailing newlines"""
        nt.assert_equal(self.results[0], "/tmp/foo",
                        msg="Trailing newline not removed!")

    def test_spaces(self):
        """core.parse_listfile: Remove trailing spaces"""
        nt.assert_equal(self.results[1], "/tmp/foo",
                        msg="Trailing spaces not removed!")

    def test_tabs(self):
        """core.parse_listfile: Remove trailing tabs"""
        nt.assert_equal(self.results[2], "/tmp/foo",
                        msg="Trailing tabs not removed!")


def test_parse_listfile_tilde():
    """ Test that parse_listfile properly expands tildes

    According to the python docs for python 2.7
    (http://docs.python.org/2/library/os.path.html#module-os.path), both
    os.path.expanduser and os.path.expandvars work on both *nix systems (Linux,
    *BSD, OSX) and Windows.

    """
    contents = "~/foo\n"

    with utils.with_tempfile(contents) as tfile:
        results = core.parse_listfile(tfile)

    assert results[0] == os.path.expandvars("$HOME/foo")


@_save_core_config
def test_xdg_config_home():
    """ get_config() finds $XDG_CONFIG_HOME/piglit.conf """
    with utils.tempdir() as tdir:
        os.environ['XDG_CONFIG_HOME'] = tdir
        with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
            f.write(_CONF_FILE)
        core.get_config()

    nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
           msg='$XDG_CONFIG_HOME not found')


@_save_core_config
def test_config_home_fallback():
    """ get_config() finds $HOME/.config/piglit.conf """
    with utils.tempdir() as tdir:
        os.environ['HOME'] = tdir
        os.mkdir(os.path.join(tdir, '.config'))
        with open(os.path.join(tdir, '.config/piglit.conf'), 'w') as f:
            f.write(_CONF_FILE)
        core.get_config()

    nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
           msg='$HOME/.config/piglit.conf not found')


@_save_core_config
@utils.test_in_tempdir
def test_local():
    """ get_config() finds ./piglit.conf """
    with utils.tempdir() as tdir:
        os.chdir(tdir)

        with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
            f.write(_CONF_FILE)

        core.get_config()

    nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
           msg='./piglit.conf not found')


@_save_core_config
def test_piglit_root():
    """ get_config() finds "piglit root"/piglit.conf """
    with open('piglit.conf', 'w') as f:
        f.write(_CONF_FILE)
    return_dir = os.getcwd()
    try:
        os.chdir('..')
        core.get_config()
    finally:
        os.chdir(return_dir)
        os.unlink('piglit.conf')

    nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
           msg='$PIGLIT_ROOT not found')
