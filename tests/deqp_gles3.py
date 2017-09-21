# Copyright 2014, 2015 Intel Corporation
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

"""Piglit integrations for dEQP GLES3 tests."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import warnings

from framework.core import PIGLIT_CONFIG
from framework.test import deqp
from framework.options import OPTIONS

__all__ = ['profile']


def _deprecated_get(env_, conf_, dep_env=None, dep_conf=('', ''), **kwargs):
    """Attempt to get deprecated values, then modern vaules.

    If a deprecated value is found give the user a warning, this uses
    deqp_get_option internally for both the deprecated and undeprecated paths,
    but prefers the old version and issues a warning if they are encountered.
    The old version is looked up unconditionally, if it is not found then the
    new version will be looked up unconditionally, with the default and
    requires keywords (which the first will not have).
    """
    val = None
    if dep_env is not None and dep_conf is not None:
        val = deqp.get_option(dep_env, dep_conf)

        if dep_env is not None and os.environ.get(dep_env) is not None:
            # see if the old environment variable was set, if it is uses it,
            # and give a deprecation warning
            warnings.warn(
                '{} has been replaced by {} and will be removed. You should '
                'update any scripts using the old environment variable'.format(
                    dep_env, env_))
        elif dep_conf != ('', '') and PIGLIT_CONFIG.has_option(*dep_conf):
            warnings.warn(
                '{} has been replaced by {} and will be removed. You should '
                'update any scripts using the old conf variable'.format(
                    ':'.join(dep_conf), ':'.join(conf_)))

    return val if val is not None else deqp.get_option(env_, conf_, **kwargs)


_DEQP_GLES3_BIN = _deprecated_get('PIGLIT_DEQP_GLES3_BIN',
                                  ('deqp-gles3', 'bin'),
                                  required=True,
                                  dep_env='PIGLIT_DEQP_GLES3_EXE',
                                  dep_conf=('deqp-gles3', 'exe'))

_DEQP_MUSTPASS = _deprecated_get('PIGLIT_DEQP3_MUSTPASS',
                                 ('deqp-gles3', 'mustpasslist'),
                                 dep_env='PIGLIT_DEQP_MUSTPASS',
                                 required=OPTIONS.deqp_mustpass)

_EXTRA_ARGS = deqp.get_option('PIGLIT_DEQP_GLES3_EXTRA_ARGS',
                              ('deqp-gles3', 'extra_args'),
                              default='').split()


class DEQPGLES3Test(deqp.DEQPBaseTest):
    deqp_bin = _DEQP_GLES3_BIN

    @property
    def extra_args(self):
        return super(DEQPGLES3Test, self).extra_args + \
            [x for x in _EXTRA_ARGS if not x.startswith('--deqp-case')]


profile = deqp.make_profile(  # pylint: disable=invalid-name
    deqp.select_source(_DEQP_GLES3_BIN, 'dEQP-GLES3-cases.txt', _DEQP_MUSTPASS,
                       _EXTRA_ARGS),
    DEQPGLES3Test)
