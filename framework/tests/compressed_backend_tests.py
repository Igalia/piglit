# Copyright (c) 2015 Intel Corporation

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

"""Tests for compressed backends.

This modules tests for compression support. Most of the tests are fairly basic,
aiming to verify that compression and decompression works as expected.

"""

from __future__ import print_function, absolute_import, division
import os
import functools

import nose.tools as nt
from nose.plugins.skip import SkipTest

from framework.tests import utils
from framework.backends import compression, abstract

# pylint: disable=line-too-long,protected-access

# Helpers


class _TestBackend(abstract.FileBackend):
    """A class for testing backend compression."""
    _file_extension = 'test_extension'

    def initialize(self, *args, **kwargs):  # pylint: disable=unused-argument
        os.mkdir(os.path.join(self._dest, 'tests'))

    def finalize(self, *args, **kwargs): # pylint: disable=unused-argument
        tests = os.path.join(self._dest, 'tests')
        with self._write_final(os.path.join(self._dest, 'results.txt')) as f:
            for file_ in os.listdir(tests):
                with open(os.path.join(tests, file_), 'r') as t:
                    f.write(t.read())

    @staticmethod
    def _write(f, name, data):  # pylint: disable=arguments-differ
        f.write('{}: {}'.format(name, data))


def _add_compression(value):
    """Decorator that temporarily adds support for a compression method."""

    def _wrapper(func):
        """The actual wrapper."""

        @functools.wraps(func)
        def _inner(*args, **kwargs):
            """The function called."""
            compression.COMPRESSORS[value] = None
            compression.DECOMPRESSORS[value] = None

            try:
                func(*args, **kwargs)
            finally:
                del compression.COMPRESSORS[value]
                del compression.DECOMPRESSORS[value]

        return _inner

    return _wrapper


def _test_compressor(mode):
    """Helper to simplify testing compressors."""
    func = compression.COMPRESSORS[mode]
    with utils.tempdir() as t:
        with func(os.path.join(t, 'file')) as f:
            f.write('foo')


def _test_decompressor(mode):
    """helper to simplify testing decompressors."""
    func = compression.COMPRESSORS[mode]
    dec = compression.DECOMPRESSORS[mode]

    with utils.tempdir() as t:
        path = os.path.join(t, 'file')

        with func(path) as f:
            f.write('foo')

        with dec(path) as f:
            nt.eq_(f.read(), 'foo')


def _test_extension():
    """Create an final file and return the extension."""
    with utils.tempdir() as d:
        obj = _TestBackend(d)
        obj.initialize()
        with obj.write_test('foo') as t:
            t({'result': 'foo'})

        obj.finalize()

        for each in os.listdir(d):
            if each.startswith('results.txt'):
                name, ext = os.path.splitext(each)
                if name.endswith('.'):
                    raise utils.TestFailure(
                        'extra trailing "." in name "{}"'.format(name))
                break
        else:
            raise utils.TestFailure('No results file generated')

    return ext


# Tests


@utils.no_error
def test_compress_none():
    """framework.backends.compression: can compress to 'none'"""
    _test_compressor('none')


def test_decompress_none():
    """framework.backends.compression: can decompress from 'none'"""
    _test_decompressor('none')



@_add_compression('foobar')
@utils.set_env(PIGLIT_COMPRESSION='foobar')
def testget_mode_env():
    """framework.backends.compression.get_mode: uses PIGlIT_COMPRESSION environment variable"""
    nt.eq_(compression.get_mode(), 'foobar')


@_add_compression('foobar')
@utils.set_env(PIGLIT_COMPRESSION=None)
@utils.set_piglit_conf(('core', 'compression', 'foobar'))
def testget_mode_piglit_conf():
    """framework.backends.compression.get_mode: uses piglit.conf [core]:compression value if env is unset"""
    nt.eq_(compression.get_mode(), 'foobar')


@utils.set_env(PIGLIT_COMPRESSION=None)
@utils.set_piglit_conf(('core', 'compression', None))
def testget_mode_default():
    """framework.backends.compression.get_mode: uses DEFAULT if env and piglit.conf are unset"""
    nt.eq_(compression.get_mode(), compression.DEFAULT)


@utils.no_error
def test_compress_gz():
    """framework.backends.compression: can compress to 'gz'"""
    _test_compressor('gz')


def test_decompress_gz():
    """framework.backends.compression: can decompress from 'gz'"""
    _test_decompressor('gz')


@utils.set_env(PIGLIT_COMPRESSION='gz')
def test_gz_output():
    """framework.backends: when using gz compression a gz file is created"""
    nt.eq_(_test_extension(), '.gz')


@utils.no_error
def test_compress_bz2():
    """framework.backends.compression: can compress to 'bz2'"""
    _test_compressor('bz2')


def test_decompress_bz2():
    """framework.backends.compression: can decompress from 'bz2'"""
    _test_decompressor('bz2')


@utils.set_env(PIGLIT_COMPRESSION='bz2')
def test_bz2_output():
    """framework.backends: when using bz2 compression a bz2 file is created"""
    nt.eq_(_test_extension(), '.bz2')


@utils.no_error
def test_compress_xz():
    """framework.backends.compression: can compress to 'xz'"""
    _test_compressor('xz')


def test_decompress_xz():
    """framework.backends.compression: can decompress from 'xz'"""
    _test_decompressor('xz')


@utils.set_env(PIGLIT_COMPRESSION='xz')
def test_xz_output():
    """framework.backends: when using xz compression a xz file is created"""
    nt.eq_(_test_extension(), '.xz')


@_add_compression('foobar')
@utils.set_env(PIGLIT_COMPRESSION=None)
@utils.set_piglit_conf(('core', 'compression', 'foobar'))
def test_update_piglit_conf():
    """framework.backends.compression: The compression mode honors updates to piglit.conf.

    the values in piglit.conf are subject to change. And the default
    compression mode needs to be changed with them.

    """
    nt.eq_(compression.get_mode(), 'foobar')


@utils.set_env(PIGLIT_COMPRESSION='xz')
@utils.test_in_tempdir
def test_xz_shell_override():
    """framework.backends.compression: the xz shell utility path can overwrite"""
    # TODO: this test will not be required by python3, where the builtin lzma
    # module replaces all of this.
    try:
        import backports.lzma  # pylint: disable=unused-variable
    except ImportError:
        pass
    else:
        raise SkipTest('Test requires shell path, not backports.lzma path.')

    with open('foo.json.xz', 'w') as f:
        f.write('foo')

    with compression.COMPRESSORS['xz']('foo.json') as f:
        f.write('foobar')


@utils.set_piglit_conf(('core', 'compression', 'bz2'))
def test_write_compressed_one_suffix_bz2():
    """backends.abstract.write_compressed: bz2 Does not duplicate compression suffixes
    """
    with utils.tempdir() as d:
        with abstract.write_compressed(os.path.join(d, 'results.txt.bz2')) as f:
            f.write('foo')

        nt.eq_(os.listdir(d)[0], 'results.txt.bz2')


@utils.set_piglit_conf(('core', 'compression', 'gz'))
def test_write_compressed_one_suffix_gz():
    """backends.abstract.write_compressed: gz Does not duplicate compression suffixes
    """
    with utils.tempdir() as d:
        with abstract.write_compressed(os.path.join(d, 'results.txt.gz')) as f:
            f.write('foo')

        nt.eq_(os.listdir(d)[0], 'results.txt.gz')


@utils.set_piglit_conf(('core', 'compression', 'gz'))
def test_write_compressed_one_suffix_mixed():
    """backends.abstract.write_compressed: does not generate two different compression suffixes
    """
    with utils.tempdir() as d:
        with abstract.write_compressed(os.path.join(d, 'results.txt.bz2')) as f:
            f.write('foo')

        nt.eq_(os.listdir(d)[0], 'results.txt.gz')
