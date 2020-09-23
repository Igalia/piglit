# coding=utf-8
# Copyright (c) 2018-2019 Intel Corporation

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

By default this will run GLES2, GLES3 and GLES31 test cases. Those
desiring to run only a subset of them should consider using the -t or
-x options to include or exclude tests.

For example:
./piglit run gtf_gles -c foo -t ES3- would run only ES3 tests (note
the dash to exclude ES31 tests)

This integration requires some configuration in piglit.conf, or the
use of environment variables.

In piglit.conf one should set the following:
[gtf_gles]:bin -- Path to the glcts binary
[gtf_gles]:extra_args -- any extra arguments to be passed to cts
(optional)

Alternatively (or in addition, since environment variables have
precedence), one could set:
PIGLIT_GTF_GLES_BIN -- environment equivalent of [gtf_gles]:bin
PIGLIT_GTF_GLES_EXTRA_ARGS -- environment equivalent of
[gtf_gles]:extra_args

"""

import itertools

from framework import core
from framework.test import deqp

__all__ = ['profile']

_GTF_BIN = core.get_option('PIGLIT_GTF_GLES_BIN', ('gtf_gles', 'bin'),
                           required=True)

_EXTRA_ARGS = core.get_option('PIGLIT_GTF_GLES_EXTRA_ARGS', ('gtf_gles', 'extra_args'),
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
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GLES2-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GLES3-cases.txt', _EXTRA_ARGS)),
        deqp.iter_deqp_test_cases(
            deqp.gen_caselist_txt(_GTF_BIN, 'GTF-GLES31-cases.txt', _EXTRA_ARGS)),
    ),
    DEQPGTFTest)
