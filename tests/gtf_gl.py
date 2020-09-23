# coding=utf-8
# Copyright (c) 2018, 2019 Intel Corporation

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

"""Piglit integration for KC CTS' GTF tests

By default this will run GL30, GL31, GL32, GL33, GL40, GL41, GL42,
GL43, GL44, GL45 and GL46 test cases. Those desiring to run only a
subset of them should consider using the -t or -x options to include
or exclude tests.

For example:
./piglit run gtf_gl -c foo -t GL30- would run only GL30 tests

This integration requires some configuration in piglit.conf, or the use of
environment variables.

In piglit.conf one should set the following:
[gtf_gl]:bin -- Path to the glcts binary
[gtf_gl]:extra_args -- any extra arguments to be passed to cts (optional)

Alternatively (or in addition, since environment variables have precedence),
one could set:
PIGLIT_GTF_GL_BIN -- environment equivalent of [gtf_gl]:bin
PIGLIT_GTF_GL_EXTRA_ARGS -- environment equivalent of [gtf_gl]:extra_args

"""

import itertools

from framework import core
from framework.test import deqp

__all__ = ['profile']

_GTF_BIN = core.get_option('PIGLIT_GTF_GL_BIN', ('gtf_gl', 'bin'),
                           required=True)

_EXTRA_ARGS = core.get_option('PIGLIT_GTF_GL_EXTRA_ARGS', ('gtf_gl', 'extra_args'),
                              default='').split()


class DEQPGTFTest(deqp.DEQPBaseTest):
    deqp_bin = _GTF_BIN

    @property
    def extra_args(self):
        return super(DEQPGTFTest, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]

# Add all of the suites by default, users can use filters to remove them.
profile = deqp.make_profile(  # pylint: disable=invalid-name
    itertools.chain(
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL30-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL31-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL32-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL33-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL40-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL41-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL42-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL43-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL44-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL45-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GL46-cases.txt', _EXTRA_ARGS)),
    ),
    DEQPGTFTest)
