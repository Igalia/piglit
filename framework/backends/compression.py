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

"""Compression support for backends.

This includes both compression and decompression support.

The primary way to interact with this module should be through the use of the
COMPRESSORS and DECOMPRESSORS constants.

These constants provide a dictionary
mapping text representations of the compression methods to functions that
should be called as context managers using the filename as the only argument.

For example:
with COMPRESSORS['none'] as f:
    f.write('foobar')

COMPRESSOR provides a convenience method for getting the default compressor,
although COMPRESSORS is available, for more advanced uses.

They should always take unicode objects. It is up to the caller to ensure that
they're passing unicode and not bytes.

"""

from __future__ import print_function, absolute_import, division
import functools
import gzip
import os

from framework import exceptions
from framework.core import PIGLIT_CONFIG

COMPRESSION_SUFFIXES = ['.gz']

DEFAULT = 'gz'

COMPRESSORS = {
    'gz': functools.partial(gzip.open, mode='w'),
    'none': functools.partial(open, mode='w'),
}

DECOMPRESSORS = {
    'gz': functools.partial(gzip.open, mode='r'),
    'none': functools.partial(open, mode='r'),
}


def _set_mode():
    """Set the compression mode.

    Try the environment variable PIGLIT_COMPRESSION; then check the
    PIGLIT_CONFIG section 'core', option 'compression'; finally fall back to
    DEFAULT.

    """
    method = (os.environ.get('PIGLIT_COMPRESSION') or
              PIGLIT_CONFIG.safe_get('core', 'compression') or
              DEFAULT)

    if method not in COMPRESSORS:
        raise exceptions.PiglitFatalError(
            'unsupported compression method {}'.format(method))
    if method not in DECOMPRESSORS:
        raise exceptions.PiglitFatalError(
            'unsupported decompression method {}'.format(method))

    return method


MODE = _set_mode()

COMPRESSOR = COMPRESSORS[MODE]
