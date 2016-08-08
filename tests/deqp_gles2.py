# Copyright 2015 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""Piglit integrations for dEQP GLES2 tests."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

from framework.test import deqp
from framework.options import OPTIONS

__all__ = ['profile']

# Path to the deqp-gles2 executable.
_DEQP_GLES2_BIN = deqp.get_option('PIGLIT_DEQP_GLES2_BIN',
                                  ('deqp-gles2', 'bin'),
                                  required=True)

_DEQP_MUSTPASS = deqp.get_option('PIGLIT_DEQP2_MUSTPASS',
                                 ('deqp-gles2', 'mustpasslist'),
                                 required=OPTIONS.deqp_mustpass)

_EXTRA_ARGS = deqp.get_option('PIGLIT_DEQP_GLES2_EXTRA_ARGS',
                              ('deqp-gles2', 'extra_args'),
                              default='').split()


class DEQPGLES2Test(deqp.DEQPSingleTest):
    deqp_bin = _DEQP_GLES2_BIN

    @property
    def extra_args(self):
        return super(DEQPGLES2Test, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]



profile = deqp.make_profile(  # pylint: disable=invalid-name
    deqp.select_source(_DEQP_GLES2_BIN, 'dEQP-GLES2-cases.txt', _DEQP_MUSTPASS,
                       _EXTRA_ARGS),
    DEQPGLES2Test)
