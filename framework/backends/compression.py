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

"""Compression support for backends.

This includes both compression and decompression support.

This provides a low level interface of dictionaries, COMPRESSORS and
DECOMPRESSORS, which use compression modes ('bz2', 'gz', 'xz', 'none') to
provide open-like functions with correct mode settings for writing or reading,
respectively.

They should always take unicode (str in python 3.x) objects. It is up to the
caller to ensure that they're passing unicode and not bytes.

A helper, get_mode(), is provided to return the user selected mode (it will try
the PIGLIT_COMPRESSION environment variable, then the piglit.conf
[core]:compression key, and finally the value of compression.DEFAULT). This is
the best way to get a compressor.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import bz2
import errno
import functools
import gzip
import os
import subprocess
import contextlib

import six
from six.moves import cStringIO as StringIO

from framework import exceptions, compat
from framework.core import PIGLIT_CONFIG

__all__ = [
    'UnsupportedCompressor',
    'COMPRESSORS',
    'DECOMPRESSORS',
    'get_mode',
]


@compat.python_2_unicode_compatible
class UnsupportedCompressor(exceptions.PiglitInternalError):
    def __init__(self, method, *args, **kwargs):
        super(UnsupportedCompressor, self).__init__(*args, **kwargs)
        self.__method = method

    def __str__(self):
        return u'unsupported compression method {}'.format(self.__method)



DEFAULT = 'bz2'

if six.PY2:
    COMPRESSION_SUFFIXES = ['.gz', '.bz2']
    COMPRESSORS = {
        'bz2': functools.partial(bz2.BZ2File, mode='w'),
        'gz': functools.partial(gzip.open, mode='w'),
        'none': functools.partial(open, mode='w'),
    }

    DECOMPRESSORS = {
        'bz2': functools.partial(bz2.BZ2File, mode='r'),
        'gz': functools.partial(gzip.open, mode='r'),
        'none': functools.partial(open, mode='r'),
    }

    # First try to use backports.lzma, that's the easiest solution. If that
    # fails then go to trying the shell. If that fails then piglit won't have
    # xz support, and will raise an error if xz is used
    try:
        import backports.lzma  # pylint: disable=wrong-import-position

        COMPRESSORS['xz'] = functools.partial(backports.lzma.open, mode='w')
        DECOMPRESSORS['xz'] = functools.partial(backports.lzma.open, mode='r')
        COMPRESSION_SUFFIXES += ['.xz']
    except ImportError:
        try:
            with open(os.devnull, 'w') as d:
                subprocess.check_call(['xz', '--help'], stdout=d, stderr=d)
        except OSError:
            pass
        else:

            @contextlib.contextmanager
            def _compress_xz(filename):
                """Emulates an open function in write mode for xz.

                Python 2.x doesn't support xz, but it's dang useful. This
                function calls out to the shell and tries to use xz from the
                environment to get xz compression.

                This obviously won't work without a working xz binary.

                This function tries to emulate the default values of the lzma
                module in python3 as much as possible

                """
                if filename.endswith('.xz'):
                    filename = filename[:-3]

                with open(filename, 'w') as f:
                    yield f

                try:
                    with open(os.devnull, 'w') as null:
                        subprocess.check_call(
                            ['xz', '--compress', '-9', '--force', filename],
                            stderr=null)
                except OSError as e:
                    if e.errno == errno.ENOENT:
                        raise exceptions.PiglitFatalError(
                            'No xz binary available')
                    raise

            @contextlib.contextmanager
            def _decompress_xz(filename):
                """Eumlates an option function in read mode for xz.

                See the comment in _compress_xz for more information.

                This function tries to emulate the lzma module as much as
                possible

                """
                if not filename.endswith('.xz'):
                    filename = '{}.xz'.format(filename)

                try:
                    with open(os.devnull, 'w') as null:
                        string = subprocess.check_output(
                            ['xz', '--decompress', '--stdout', filename],
                            stderr=null)
                except OSError as e:
                    if e.errno == errno.ENOENT:
                        raise exceptions.PiglitFatalError(
                            'No xz binary available')
                    raise

                # We need a file-like object, so the contents must be placed in
                # a StringIO object.
                io = StringIO()
                io.write(string)
                io.seek(0)

                yield io

                io.close()

            COMPRESSORS['xz'] = _compress_xz
            DECOMPRESSORS['xz'] = _decompress_xz
            COMPRESSION_SUFFIXES += ['.xz']
else:
    # In the case of python 3 this all just works, no monkeying around with
    # imports and fallbacks. just import the right modules and go

    import lzma  # pylint: disable=wrong-import-position,wrong-import-order

    COMPRESSION_SUFFIXES = ['.gz', '.bz2', '.xz']

    COMPRESSORS = {
        'bz2': functools.partial(bz2.open, mode='wt'),
        'gz': functools.partial(gzip.open, mode='wt'),
        'none': functools.partial(open, mode='w'),
        'xz': functools.partial(lzma.open, mode='wt'),
    }

    DECOMPRESSORS = {
        'bz2': functools.partial(bz2.open, mode='rt'),
        'gz': functools.partial(gzip.open, mode='rt'),
        'none': functools.partial(open, mode='r'),
        'xz': functools.partial(lzma.open, mode='rt'),
    }


def get_mode():
    """Return the key value of the correct compressor to use.

    Try the environment variable PIGLIT_COMPRESSION; then check the
    PIGLIT_CONFIG section 'core', option 'compression'; finally fall back to
    DEFAULT.

    This will raise an UnsupportedCompressionError if there isn't a compressor
    for that mode. It is the job of the caller to handle this exceptions

    """
    # This is provided as a function rather than a constant because as a
    # function it can honor changes to the PIGLIT_CONFIG instance, or the
    # PIGLIT_COMPRESSION environment variable.

    method = (os.environ.get('PIGLIT_COMPRESSION') or
              PIGLIT_CONFIG.safe_get('core', 'compression') or
              DEFAULT)

    if method not in COMPRESSORS:
        raise UnsupportedCompressor(method)

    return method
