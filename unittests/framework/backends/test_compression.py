# coding=utf-8
# Copyright (c) 2015-2016 Intel Corporation

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

"""Tests for compression in file backends."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools
import os
import subprocess
try:
    import mock
except ImportError:
    from unittest import mock

import pytest
import six

from framework import core
from framework.backends import abstract
from framework.backends import compression

from .. import skip

# pylint: disable=no-self-use,redefined-outer-name

# Helpers


def _has_lzma():
    """Try to import backports.lzma, and return True if it exists, False if it
    doesn't.
    """
    try:
        import backports.lzma  # pylint: disable=unused-variable
    except ImportError:
        return False
    return True


def _has_xz_bin():
    """Check for an xz binary."""
    try:
        subprocess.check_call(['xz', '--help'])
    except subprocess.CalledProcessError:
        return False
    except OSError as e:  # pylint: disable=undefined-variable
        if e.errno != 2:
            raise
        return False
    return True


requires_lzma = pytest.mark.skipif(  # pylint: disable=invalid-name
    six.PY2 and not _has_lzma(),
    reason="Python 2.x requires backports.lzma to run this test.")

requires_xz_bin = pytest.mark.skipif(  # pylint: disable=invalid-name
    _has_lzma() or not _has_xz_bin(),
    reason="Python 2.x requires xz binary to run this test.")

requires_any_lzma = pytest.mark.skipif(  # pylint: disable=invalid-name
    six.PY2 and not (_has_lzma() or _has_xz_bin()),
    reason="Python 2.x requires some form of xz compression to run this test.")


@pytest.yield_fixture
def env():
    with mock.patch.dict('os.environ'):
        yield os.environ

@pytest.yield_fixture
def config():
    with mock.patch('framework.backends.compression.PIGLIT_CONFIG',
                    new_callable=core.PiglitConfig) as conf:
        conf.add_section('core')
        yield conf

@pytest.yield_fixture
def compressor():
    """Workaround for yield_fixture not working with classes."""
    class Compressor(object):
        """Fixture to control the compressor/decompressor objects."""

        def __init__(self):
            self._mock_c = mock.patch.dict(
                'framework.backends.compression.COMPRESSORS', clear=False)
            self._mock_d = mock.patch.dict(
                'framework.backends.compression.DECOMPRESSORS', clear=False)

        def __enter__(self):
            self._mock_c.start()
            self._mock_d.start()
            return self

        def __exit__(self, type_, value, traceback):
            self._mock_c.stop()
            self._mock_d.stop()

        def add(self, name, func):
            assert name not in compression.COMPRESSORS
            assert name not in compression.DECOMPRESSORS
            compression.COMPRESSORS[name] = func
            compression.DECOMPRESSORS[name] = func

        def rm(self, name):  # pylint: disable=invalid-name
            assert name in compression.COMPRESSORS
            assert name in compression.DECOMPRESSORS
            del compression.COMPRESSORS[name]
            del compression.DECOMPRESSORS[name]

    with Compressor() as c:
        yield c


# Tests


@pytest.mark.parametrize("mode", ['none', 'bz2', 'gz', requires_lzma('xz')])
def test_compress(mode, tmpdir):
    """Test that each compressor that we want works.

    These only check using modules, that means on python2 this test will skip
    unless backports.lzma is available, and it will not test the the xz binary
    path.
    """
    func = compression.COMPRESSORS[mode]
    testfile = tmpdir.join('test')
    with func(six.text_type(testfile)) as f:
        f.write('foo')


@pytest.mark.parametrize("mode", ['none', 'bz2', 'gz', requires_lzma('xz')])
def test_decompress(mode, tmpdir):
    """Test that each supported decompressor works.

    See the docstring in test_compress.
    """
    comp = compression.COMPRESSORS[mode]
    dec = compression.DECOMPRESSORS[mode]
    testfile = tmpdir.join('test')

    with comp(six.text_type(testfile)) as f:
        f.write('foo')

    with dec(six.text_type(testfile)) as f:
        actual = f.read()

    assert actual == 'foo'


@skip.posix
@skip.PY3
@requires_xz_bin
class TestXZBin(object):
    """Tests for the xz bin path on python2.x."""

    def test_compress_xz_bin(self, tmpdir):
        """Test python2 xz compression using the xz binary."""
        func = compression.COMPRESSORS['xz']
        testfile = tmpdir.join('test')
        with func(six.text_type(testfile)) as f:
            f.write('foo')

    def test_decompress_xz_bin(self, tmpdir):
        """Test python2 xz decompression using the xz binary."""
        comp = compression.COMPRESSORS['xz']
        dec = compression.DECOMPRESSORS['xz']
        testfile = tmpdir.join('test')

        with comp(six.text_type(testfile)) as f:
            f.write('foo')

        with dec(six.text_type(testfile)) as f:
            actual = f.read()

        assert actual == 'foo'


class TestGetMode(object):
    """Tests for the compression.get_mode function."""

    def test_default(self, env, config):  # pylint: disable=unused-argument
        """When neither the config file nor the environment sets a value for
        compression the default value should be used.
        """
        env.clear()

        assert compression.get_mode() == compression.DEFAULT

    def test_env(self, env, config, compressor):
        """Test that when env doesn't have a PIGLIT_COMPRESSION environment
        varaible set, but the configuraiton has a compression method set that
        it is used.
        """
        compressor.add('foo', None)
        compressor.add('bar', None)
        config.set('core', 'compression', 'foo')
        env['PIGLIT_COMPRESSION'] = 'bar'

        assert compression.get_mode() == 'bar'

    def test_piglit_conf(self, env, config, compressor):
        """Test that when env doesn't have a PIGLIT_COMPRESSION environment
        varaible set, but the configuraiton has a compression method set that
        it is used.
        """
        compressor.add('foobar', None)
        config.set('core', 'compression', 'foobar')
        env.clear()

        assert compression.get_mode() == 'foobar'


@pytest.mark.parametrize("extension", ['bz2', 'gz', requires_any_lzma('xz')])
def test_duplicate_extensions(extension, tmpdir, config):
    """Tests that exersizes a bug that caused the compressed extension to be
    duplicated in some cases.
    """
    tmpdir.chdir()
    config.set('core', 'compression', extension)
    expected = 'results.txt.' + extension

    with abstract.write_compressed(expected) as f:
        f.write('foo')

    assert expected in os.listdir('.')


@pytest.mark.parametrize("orig,new", itertools.permutations(
    ['bz2', 'gz', 'xz'], 2))
def test_changed_extension(orig, new, tmpdir, config):
    """Tests that exersizes a bug that caused two extensions to be present if
    the compression method changed.
    """
    if 'xz' in [new, orig] and six.PY2 and not (_has_lzma() or _has_xz_bin()):
        pytest.skip("There is no xz compressor available.")

    tmpdir.chdir()
    config.set('core', 'compression', six.text_type(new))

    with abstract.write_compressed('results.txt.' + orig) as f:
        f.write('foo')

    assert 'results.txt.' + new in os.listdir('.')
    assert 'results.txt.' + orig not in os.listdir('.')
    assert 'results.txt.{}.{}'.format(orig, new) not in os.listdir('.')
