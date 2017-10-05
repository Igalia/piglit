# Copyright (c) 2017 Intel Corporation

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

"""Piglit integration for the now open sourced Khronos CTS tests being
developed at https://github.com/KhronosGroup/VK-GL-CTS

This will run GL45 test cases only.

This integration requires some configuration in piglit.conf, or the use of
environment variables.

In piglit.conf one should set the following:
[khr_gl45]:bin -- Path to the glcts binary
[khr_gl45]:extra_args -- any extra arguments to be passed to cts (optional)

Alternatively (or in addition, since environment variables have precedence),
one could set:
PIGLIT_KHR_GL_BIN -- environment equivalent of [khr_gl45]:bin
PIGLIT_KHR_GL_EXTRA_ARGS -- environment equivalent of [khr_gl45]:extra_args

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools

from framework.test import deqp
from framework.options import OPTIONS

__all__ = ['profile']

_KHR_BIN = deqp.get_option('PIGLIT_KHR_GL_BIN', ('khr_gl45', 'bin'),
                           required=True)

_KHR_MUSTPASS = deqp.get_option('PIGLIT_KHRGL45_MUSTPASS',
                                 ('khr_gl45', 'mustpasslist'),
                                 required=OPTIONS.deqp_mustpass)

_EXTRA_ARGS = deqp.get_option('PIGLIT_KHR_GL_EXTRA_ARGS', ('khr_gl45', 'extra_args'),
                              default='').split()


class DEQPKHRTest(deqp.DEQPBaseTest):
    deqp_bin = _KHR_BIN

    @property
    def extra_args(self):
        return super(DEQPKHRTest, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]

profile = deqp.make_profile(  # pylint: disable=invalid-name
    deqp.select_source(_KHR_BIN, 'KHR-GL45-cases.txt', _KHR_MUSTPASS,
                       _EXTRA_ARGS),
    DEQPKHRTest)
