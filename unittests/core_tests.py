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

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import errno
import functools
import os
import shutil
import textwrap

# There is a very high potential that one of these will raise an ImportError
# pylint: disable=import-error
try:
    from unittest import mock
except ImportError:
    import mock
# pylint: enable=import-error

import nose.tools as nt
import six
try:
    from six.moves import getcwd
except ImportError:
    # pylint: disable=no-member
    if six.PY2:
        getcwd = os.getcwdu
    elif six.PY3:
        getcwd = os.getcwd
    # pylint: enable=no-member

from framework import core, exceptions
from . import utils

# pylint: disable=line-too-long,invalid-name

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
            core.PIGLIT_CONFIG = core.PiglitConfig(allow_no_value=True)
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
            core.PIGLIT_CONFIG = core.PiglitConfig(
                allow_no_value=True)
        except Exception as e:
            raise utils.UtilsError(e)

    return inner


@utils.no_error
def test_PiglitConfig_init():
    """core.PiglitConfig: initializes"""
    core.PiglitConfig()


def test_parse_listfile_return():
    """core.parse_listfile(): returns a list-like object

    Given a file with a newline separated list of results, parse_listfile
    should return a list of files with no whitespace

    """
    contents = "/tmp/foo\n/tmp/bar\n"

    with utils.tempfile(contents) as tfile:
        results = core.parse_listfile(tfile)

    nt.ok_(isinstance(results, collections.Container))


class Test_parse_listfile_TrailingWhitespace(object):
    """Test that parse_listfile removes whitespace"""
    @classmethod
    def setup_class(cls):
        contents = "/tmp/foo\n/tmp/foo  \n/tmp/foo\t\n"
        with utils.tempfile(contents) as tfile:
            cls.results = core.parse_listfile(tfile)

    def test_newlines(self):
        """core.parse_listfile(): Remove trailing newlines"""
        nt.assert_equal(self.results[0], "/tmp/foo",
                        msg="Trailing newline not removed!")

    def test_spaces(self):
        """core.parse_listfile(): Remove trailing spaces"""
        nt.assert_equal(self.results[1], "/tmp/foo",
                        msg="Trailing spaces not removed!")

    def test_tabs(self):
        """core.parse_listfile(): Remove trailing tabs"""
        nt.assert_equal(self.results[2], "/tmp/foo",
                        msg="Trailing tabs not removed!")


def test_parse_listfile_tilde():
    """core.parse_listfile(): tildes (~) are properly expanded.

    According to the python docs for python 2.7
    (http://docs.python.org/2/library/os.path.html#module-os.path), both
    os.path.expanduser and os.path.expandvars work on both *nix systems (Linux,
    *BSD, OSX) and Windows.

    """
    contents = "~/foo\n"
    expected = os.path.expandvars("$HOME/foo")

    with utils.tempfile(contents) as tfile:
        results = core.parse_listfile(tfile)

    nt.eq_(results[0], expected,
           msg='expected: {} but got: {}'.format(expected, results[0]))


@_save_core_config
def test_xdg_config_home():
    """core.get_config() finds $XDG_CONFIG_HOME/piglit.conf"""
    with utils.tempdir() as tdir:
        os.environ['XDG_CONFIG_HOME'] = tdir
        with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
            f.write(_CONF_FILE)
        core.get_config()

    nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
           msg='$XDG_CONFIG_HOME not found')


@_save_core_config
def test_config_home_fallback():
    """core.get_config() finds $HOME/.config/piglit.conf"""
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
    """core.get_config() finds ./piglit.conf"""
    with utils.tempdir() as tdir:
        os.chdir(tdir)

        with open(os.path.join(tdir, 'piglit.conf'), 'w') as f:
            f.write(_CONF_FILE)

        core.get_config()

    nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
           msg='./piglit.conf not found')


@_save_core_config
def test_piglit_root():
    """core.get_config() finds "piglit root"/piglit.conf"""
    with open('piglit.conf', 'w') as f:
        f.write(_CONF_FILE)
    return_dir = getcwd()
    try:
        os.chdir('..')
        core.get_config()
    finally:
        os.chdir(return_dir)
        os.unlink('piglit.conf')

    nt.ok_(core.PIGLIT_CONFIG.has_section('nose-test'),
           msg='$PIGLIT_ROOT not found')


class TestPiglitConfig(object):
    """Tests for PiglitConfig methods."""
    @classmethod
    def setup_class(cls):
        cls.conf = core.PiglitConfig()
        cls.conf.add_section('set')
        cls.conf.set('set', 'options', 'bool')

    def test_safe_get_valid(self):
        """core.PiglitConfig: safe_get returns a value if its in the Config"""
        nt.assert_equal(self.conf.safe_get('set', 'options'), 'bool')

    def test_PiglitConfig_required_get_valid(self):
        """core.PiglitConfig: required_get returns a value if its in the Config
        """
        nt.assert_equal(self.conf.required_get('set', 'options'), 'bool')

    def test_safe_get_missing_option(self):
        """core.PiglitConfig: safe_get returns None if the option is missing
        """
        nt.assert_equal(self.conf.safe_get('set', 'invalid'), None)

    def test_safe_get_missing_section(self):
        """core.PiglitConfig: safe_get returns None if the section is missing
        """
        nt.assert_equal(self.conf.safe_get('invalid', 'invalid'), None)

    @nt.raises(exceptions.PiglitFatalError)
    def test_required_get_missing_option(self):
        """core.PiglitConfig: required_get raises PiglitFatalError if the option is missing
        """
        self.conf.required_get('set', 'invalid')

    @nt.raises(exceptions.PiglitFatalError)
    def test_required_get_missing_section(self):
        """core.PiglitConfig: required_get raises PiglitFatalError if the section is missing
        """
        self.conf.required_get('invalid', 'invalid')

    def test_safe_get_fallback(self):
        """core.PiglitConfig: safe_get returns the value of fallback when the section or option is missing"""
        nt.eq_(self.conf.safe_get('invalid', 'invalid', fallback='foo'), 'foo')


@utils.capture_stderr
@nt.raises(exceptions.PiglitException)
def test_check_dir_exists_fail():
    """core.check_dir: if the directory exists and failifexsits is True fail"""
    with mock.patch('framework.core.os.stat', mock.Mock(side_effect=OSError)):
        core.check_dir('foo', True)


def test_check_dir_stat_ENOENT():
    """core.check_dir: if the directory exists (ENOENT) and failifexsits is False continue"""
    with mock.patch('framework.core.os.stat',
                    mock.Mock(side_effect=OSError('foo', errno.ENOENT))):
        with mock.patch('framework.core.os.makedirs') as makedirs:
            core.check_dir('foo', False)
            nt.eq_(makedirs.called, 1)


def test_check_dir_stat_ENOTDIR():
    """core.check_dir: if a file exists (ENOTDIR) and failifexsits is False continue"""
    with mock.patch('framework.core.os.stat',
                    mock.Mock(side_effect=OSError('foo', errno.ENOTDIR))):
        with mock.patch('framework.core.os.makedirs') as makedirs:
            core.check_dir('foo', False)
            nt.eq_(makedirs.called, 1)


@utils.not_raises(OSError)
def test_check_dir_makedirs_pass():
    """core.check_dir: If makedirs fails with EEXIST pass"""
    with mock.patch('framework.core.os.stat', mock.Mock()):
        with mock.patch('framework.core.os.makedirs',
                        mock.Mock(side_effect=OSError(errno.EEXIST, 'foo'))):
            core.check_dir('foo', False)


@nt.raises(OSError)
def test_check_dir_makedirs_fail():
    """core.check_dir: If makedirs fails with any other raise"""
    with mock.patch('framework.core.os.stat', mock.Mock()):
        with mock.patch('framework.core.os.path.exists',
                        mock.Mock(return_value=False)):
            with mock.patch('framework.core.os.makedirs',
                            mock.Mock(side_effect=OSError)):
                core.check_dir('foo', False)


@nt.raises(utils.SentinalException)
def test_check_dir_handler():
    """core.check_dir: Handler is called if not failifexists."""
    with mock.patch('framework.core.os.stat',
                    mock.Mock(side_effect=OSError('foo', errno.ENOTDIR))):
        core.check_dir('foo',
                       handler=mock.Mock(side_effect=utils.SentinalException))


@utils.skip(not six.PY3, 'Test is only relevant on python 3.x')
def test_check_dir_stat_FileNotFoundError():
    """core.check_dir: FileNotFoundError is raised and failifexsits is False continue"""
    with mock.patch('framework.core.os.stat',
                    mock.Mock(side_effect=FileNotFoundError)):
        with mock.patch('framework.core.os.makedirs') as makedirs:
            core.check_dir('foo', False)
            nt.eq_(makedirs.called, 1)
