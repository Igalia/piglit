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
import bz2
import errno
import functools
import gzip
import os
import subprocess

from framework import exceptions
from framework.core import PIGLIT_CONFIG


# TODO: in python3 the bz2 module has an open function
COMPRESSION_SUFFIXES = ['.gz', '.bz2']

DEFAULT = 'bz2'

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

# TODO: in python3 there is builtin xz support, and doesn't need this madness
# First try to use backports.lzma, that's the easiest solution. If that fails
# then go to trying the shell. If that fails then piglit won't have xz support,
# and will raise an error if xz is used
try:
    import backports.lzma

    COMPRESSORS['xz'] = functools.partial(backports.lzma.open, mode='w')
    DECOMPRESSORS['xz'] = functools.partial(backports.lzma.open, mode='r')
    COMPRESSION_SUFFIXES += ['.xz']
except ImportError:
    try:
        with open(os.devnull, 'w') as d:
            subprocess.check_call(['xz'], stderr=d)
    except subprocess.CalledProcessError as e:
        if e.returncode == 1:
            import contextlib
            try:
                import cStringIO as StringIO
            except ImportError:
                import StringIO

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
                    filename = filename[:-2]

                with open(filename, 'w') as f:
                    yield f

                try:
                    subprocess.check_call(['xz', '--compress', '-9', filename])
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
                    string = subprocess.check_output(
                        ['xz', '--decompress', '--stdout', filename])
                except OSError as e:
                    if e.errno == errno.ENOENT:
                        raise exceptions.PiglitFatalError(
                            'No xz binary available')
                    raise

                # We need a file-like object, so the contents must be placed in
                # a StringIO object.
                io = StringIO.StringIO()
                io.write(string)
                io.seek(0)

                yield io

                io.close()

            COMPRESSORS['xz'] = _compress_xz
            DECOMPRESSORS['xz'] = _decompress_xz
            COMPRESSION_SUFFIXES += ['.xz']
    except OSError:
        pass


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
