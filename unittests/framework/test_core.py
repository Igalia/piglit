# coding=utf-8
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

""" Module providing tests for the core module """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import errno
import os
import textwrap

import pytest
import six

from framework import core
from framework import exceptions

from . import skip

# Making good test names often flies in the face of PEP8 recomendations, ignore
# those
# pylint: disable=invalid-name
#
# For testing purposes (and to get good names) it's useful to use classes for
# grouping tests. Often these tests share no state, which would be bad if they
# were not tests, but that's kind of the way python testing works.
# pylint: disable=no-self-use


class TestParseListfile(object):
    """Tests for core.parse_listfile."""

    def test_parse_listfile_return(self, tmpdir):
        """core.parse_listfile(): returns a list-like object.

        Given a file with a newline separated list of results, parse_listfile
        should return a list of files with no whitespace.
        """
        f = tmpdir.join('test.list')
        f.write("/tmp/foo\n/tmp/bar\n")
        results = core.parse_listfile(six.text_type(f))
        assert isinstance(results, collections.Container)

    def test_parse_listfile_whitespace(self, tmpdir):
        """parse_listfile should remove various kinds of trailing whitespace.

        It is important it doesn't touch whitespace in lines, however.
        """
        f = tmpdir.join('test.list')
        f.write(textwrap.dedent("""\
            space between
            tab\t
            space
            newline
        """))
        results = core.parse_listfile(six.text_type(f))

        assert results[0] == 'space between'
        assert results[1] == 'tab'
        assert results[2] == 'space'
        assert results[3] == 'newline'

    def test_parse_listfile_tilde_posix(self, tmpdir):
        """core.parse_listfile(): tildes (~) are properly expanded.

        According to the python docs for python 2.7
        (http://docs.python.org/2/library/os.path.html#module-os.path), both
        os.path.expanduser and os.path.expandvars work on both *nix systems
        (Linux, *BSD, OSX) and Windows.
        """
        if os.name == 'posix':
            expected = os.path.expandvars('$HOME')
        else:
            expected = os.path.expandvars('%USERPROFILE%')
        expected = os.path.normpath(os.path.join(expected, 'foo'))

        f = tmpdir.join('test.list')
        f.write("~/foo\n")
        results = core.parse_listfile(six.text_type(f))
        assert os.path.normpath(results[0]) == expected


class TestGetConfig(object):
    """Tests for core.get_config."""
    _CONF_FILE = textwrap.dedent("""\
        [nose-test]
        ; a section for testing behavior
        dir = foo""")

    @skip.linux
    def test_config_in_xdg_config_home(self, tmpdir, mocker):
        """core.get_config() finds $XDG_CONFIG_HOME/piglit.conf"""
        env = mocker.patch('framework.core.os.environ', new={})
        env['XDG_CONFIG_HOME'] = six.text_type(tmpdir)
        conf = tmpdir.join('piglit.conf')
        conf.write(self._CONF_FILE)
        core.get_config()

        assert core.PIGLIT_CONFIG.has_section('nose-test')

    @skip.linux
    def test_config_in_home_dir(self, tmpdir, mocker):
        """core.get_config() finds $HOME/.config/piglit.conf"""
        env = mocker.patch('framework.core.os.environ', new={})
        env['HOME'] = six.text_type(tmpdir)
        conf = tmpdir.join('piglit.conf')
        conf.write(self._CONF_FILE)
        core.get_config()

        assert core.PIGLIT_CONFIG.has_section('nose-test')

    def test_config_in_current(self, tmpdir, mocker):
        """core.get_config() finds ./piglit.conf"""
        mocker.patch('framework.core.os.environ', new={})
        conf = tmpdir.join('piglit.conf')
        conf.write(self._CONF_FILE)

        ret = tmpdir.chdir()
        try:
            core.get_config()
        finally:
            os.chdir(six.text_type(ret))

        assert core.PIGLIT_CONFIG.has_section('nose-test')

    def test_config_in_piglit_root(self, mocker, tmpdir):
        """core.get_config() finds "piglit root"/piglit.conf"""
        # Mock the __file__ attribute of the core module, since that's how
        # piglit decides where the root of the piglit directory is.
        mocker.patch('framework.core.__file__',
                     six.text_type(tmpdir.join('framework', 'core.py')))
        mocker.patch('framework.core.os.environ', new={})
        conf = tmpdir.join('piglit.conf')
        conf.write(self._CONF_FILE)
        core.get_config()

        assert core.PIGLIT_CONFIG.has_section('nose-test')


class TestPiglitConfig(object):
    """Tests for PiglitConfig methods."""
    @classmethod
    def setup_class(cls):
        cls.conf = core.PiglitConfig()
        cls.conf.add_section('set')
        cls.conf.set('set', 'options', 'bool')

    def test_safe_get_valid(self):
        """core.PiglitConfig: safe_get returns a value if its in the Config."""
        assert self.conf.safe_get('set', 'options') == 'bool'

    def test_PiglitConfig_required_get_valid(self):
        """core.PiglitConfig: required_get returns a value if its in the
        Config."""
        assert self.conf.required_get('set', 'options') == 'bool'

    def test_safe_get_missing_option(self):
        """core.PiglitConfig: safe_get returns None if the option is missing.
        """
        assert self.conf.safe_get('set', 'invalid') is None

    def test_safe_get_missing_section(self):
        """core.PiglitConfig: safe_get returns None if the section is missing.
        """
        assert self.conf.safe_get('invalid', 'invalid') is None

    def test_required_get_missing_option(self):
        """core.PiglitConfig: required_get raises PiglitFatalError if the
        option is missing."""
        with pytest.raises(exceptions.PiglitFatalError):
            self.conf.required_get('set', 'invalid')

    def test_required_get_missing_section(self):
        """core.PiglitConfig: required_get raises PiglitFatalError if the
        section is missing."""
        with pytest.raises(exceptions.PiglitFatalError):
            self.conf.required_get('invalid', 'invalid')

    def test_safe_get_fallback(self):
        """core.PiglitConfig: safe_get returns the value of fallback when the
        section or option is missing."""
        assert self.conf.safe_get('invalid', 'invalid', fallback='foo') == 'foo'


class TestCheckDir(object):
    """Tests for core.check_dir."""

    def test_exists_fail(self, mocker, tmpdir):
        """core.check_dir: if the directory exists and failifexsits is True
        fail."""
        tmpdir.chdir()
        mocker.patch('framework.core.os.stat', mocker.Mock(side_effect=OSError))
        with pytest.raises(exceptions.PiglitException):
            core.check_dir('foo', True)

    def test_not_exists_and_not_fail(self, mocker, tmpdir):
        """core.check_dir: if the directory doesn't exists (ENOENT) and
        failifexists is False continue."""
        tmpdir.chdir()
        mocker.patch('framework.core.os.stat',
                     mocker.Mock(side_effect=OSError('foo', errno.ENOENT)))
        makedirs = mocker.patch('framework.core.os.makedirs')

        core.check_dir('foo', False)

        assert makedirs.called == 1

    def test_exists_and_not_fail(self, mocker, tmpdir):
        """core.check_dir: If makedirs fails with EEXIST pass"""
        tmpdir.chdir()
        mocker.patch('framework.core.os.stat', mocker.Mock())
        makedirs = mocker.patch(
            'framework.core.os.makedirs',
            mocker.Mock(side_effect=OSError(errno.EEXIST, 'foo')))

        core.check_dir('foo', False)

        assert makedirs.called == 0

    def test_makedirs_fail(self, mocker, tmpdir):
        """core.check_dir: If makedirs fails with any other raise that error."""
        tmpdir.chdir()
        mocker.patch('framework.core.os.makedirs',
                     mocker.Mock(side_effect=OSError))

        with pytest.raises(OSError):
            core.check_dir('foo', False)

    def test_handler(self, mocker, tmpdir):
        """core.check_dir: Handler is called if not failifexists."""
        class Sentinel(Exception):
            pass

        tmpdir.chdir()

        mocker.patch('framework.core.os.stat',
                     mocker.Mock(side_effect=OSError('foo', errno.ENOTDIR)))
        with pytest.raises(Sentinel):
            core.check_dir('foo', handler=mocker.Mock(side_effect=Sentinel))

    @skip.PY2
    def test_stat_FileNotFoundError(self, mocker, tmpdir):
        """core.check_dir: FileNotFoundError is raised and failifexsits is
        False continue."""
        tmpdir.chdir()
        mocker.patch('framework.core.os.stat',
                     mocker.Mock(side_effect=FileNotFoundError))
        makedirs = mocker.patch('framework.core.os.makedirs')

        core.check_dir('foo', False)

        assert makedirs.called == 1
