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

By default this will run GL30, GL31, GL32, GL33, GL40, GL41, GL42, GL43, GL44
and GL45 test cases. Those desiring to run only a subset of them should consider
using the -t or -x options to include or exclude tests.

For example:
./piglit run khr_gl -c foo -t GL30- would run only GL30 tests

This integration requires some configuration in piglit.conf, or the use of
environment variables.

In piglit.conf one should set the following:
[khr_gl]:bin -- Path to the glcts binary
[khr_gl]:extra_args -- any extra arguments to be passed to cts (optional)

Alternatively (or in addition, since environment variables have precedence),
one could set:
PIGLIT_KHR_GL_BIN -- environment equivalent of [khr_gl]:bin
PIGLIT_KHR_GL_EXTRA_ARGS -- environment equivalent of [khr_gl]:extra_args

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools

from framework.test import deqp

__all__ = ['profile']

_KHR_BIN = deqp.get_option('PIGLIT_KHR_GL_BIN', ('khr_gl', 'bin'),
                           required=True)

_EXTRA_ARGS = deqp.get_option('PIGLIT_KHR_GL_EXTRA_ARGS', ('khr_gl', 'extra_args'),
                              default='').split()


class DEQPKHRTest(deqp.DEQPBaseTest):
    deqp_bin = _KHR_BIN

    @property
    def extra_args(self):
        return super(DEQPKHRTest, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]

# Add all of the suites by default, users can use filters to remove them.
profile = deqp.make_profile(  # pylint: disable=invalid-name
    itertools.chain(
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL30-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL31-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL32-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL33-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL40-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL41-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL42-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL43-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL44-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL45-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_KHR_BIN, 'KHR-GL46-cases.txt', _EXTRA_ARGS)),
    ),
    DEQPKHRTest)
