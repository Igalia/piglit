# encoding=utf-8
# Copyright © 2015-2016 Intel Corporation
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

"""Piglit integrations for dEQP EGL tests."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

from framework.test import deqp

__all__ = ['profile']

_EGL_BIN = deqp.get_option('PIGLIT_DEQP_EGL_BIN',
                           ('deqp-egl', 'bin'),
                           required=True)

_EXTRA_ARGS = deqp.get_option('PIGLIT_DEQP_EGL_EXTRA_ARGS',
                              ('deqp-egl', 'extra_args'),
                              default='').split()


class DEQPEGLTest(deqp.DEQPSingleTest):
    deqp_bin = _EGL_BIN

    @property
    def extra_args(self):
        return super(DEQPEGLTest, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]


profile = deqp.make_profile(  # pylint: disable=invalid-name
    deqp.iter_deqp_test_cases(
        deqp.gen_caselist_txt(_EGL_BIN, 'dEQP-EGL-cases.txt',
                              _EXTRA_ARGS)),
    DEQPEGLTest)
