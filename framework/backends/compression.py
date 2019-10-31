# coding=utf-8
# Copyright (c) 2015-2016, 2019 Intel Corporation

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

import bz2
import contextlib
import errno
import functools
import gzip
import io
import lzma
import os
import subprocess

from framework import exceptions
from framework.core import PIGLIT_CONFIG

__all__ = [
    'UnsupportedCompressor',
    'COMPRESSORS',
    'DECOMPRESSORS',
    'get_mode',
]


class UnsupportedCompressor(exceptions.PiglitInternalError):
    def __init__(self, method, *args, **kwargs):
        super(UnsupportedCompressor, self).__init__(*args, **kwargs)
        self.__method = method

    def __str__(self):
        return 'unsupported compression method {}'.format(self.__method)



DEFAULT = 'bz2'

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
