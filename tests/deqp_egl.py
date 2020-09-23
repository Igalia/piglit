# encoding=utf-8
# Copyright © 2015-2016, 2019 Intel Corporation
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

from framework import core
from framework.test import deqp
from framework.options import OPTIONS

__all__ = ['profile']

_EGL_BIN = core.get_option('PIGLIT_DEQP_EGL_BIN',
                           ('deqp-egl', 'bin'),
                           required=True)

_DEQP_MUSTPASS = core.get_option('PIGLIT_DEQP_EGL_MUSTPASS',
                                 ('deqp-egl', 'mustpasslist'),
                                 required=OPTIONS.deqp_mustpass)

_EXTRA_ARGS = core.get_option('PIGLIT_DEQP_EGL_EXTRA_ARGS',
                              ('deqp-egl', 'extra_args'),
                              default='').split()


class DEQPEGLTest(deqp.DEQPBaseTest):
    deqp_bin = _EGL_BIN

    @property
    def extra_args(self):
        return super(DEQPEGLTest, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]


profile = deqp.make_profile(  # pylint: disable=invalid-name
    deqp.select_source(_EGL_BIN, 'dEQP-EGL-cases.txt', _DEQP_MUSTPASS,
                       _EXTRA_ARGS),
    DEQPEGLTest)
