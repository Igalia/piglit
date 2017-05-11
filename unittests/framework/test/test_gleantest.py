# Copyright (c) 2014 Intel Corporation

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

""" Tests for the glean class. Requires Nose """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import pytest

from framework.options import _Options as Options
from framework import status
from framework.test import GleanTest
from framework.test.base import TestIsSkip as _TestIsSkip  # make py.test happy

# pylint: disable=invalid-name
# pylint: disable=protected-access


def test_GLOBAL_PARAMS_assignment():
    """test.gleantest.GleanTest: GLOBAL_PARAMS apply to instances created
    after GLABL_PARAMS is set.

    Specifically this tests for a bug where GLOBAL_PARAMS only affected
    instances of GleanTest created after GLOBAL_PARAMS were set, so changing the
    GLOBAL_PARAMS value had unexpected results.

    If this test passes the GleanTest.command attributes will be the same in
    the instance created before the GLOBAL_PARAMS assignment and the one created
    after. A failure means the that GLOBAL_PARAMS are not being added to tests
    initialized before it is set.

    """
    test1 = GleanTest('basic')
    GleanTest.GLOBAL_PARAMS = ['--quick']
    test2 = GleanTest('basic')
    assert test1.command == test2.command


def test_global_params_setter():
    """Values from self.GLOBAL_ARGS are not pushed into self._command by a
    setter.
    """
    test = GleanTest('basic')
    GleanTest.GLOBAL_PARAMS = ['--quick']
    test.command += '-foo'
    assert '--quick' not in test._command



def test_bad_returncode():
    """test.gleantest.GleanTest: If returncode is 0 the result is 'fail'.

    Currently glean returns 127 if piglit can't find it's libs (LD_LIBRARY_PATH
    isn't set properly), and then marks such tests as pass, when they obviously
    are not.

    """
    test = GleanTest('basic')
    test.result.returncode = 1
    test.interpret_result()
    assert test.result.result == 'fail'


def test_is_skip_not_glx(mocker):
    """test.gleantest.GleanTest.is_skip: Skips when platform isn't glx."""
    opts = mocker.patch('framework.test.gleantest.options.OPTIONS',
                        new_callable=Options)
    opts.env['PIGLIT_PLATFORM'] = 'gbm'

    test = GleanTest('foo')
    with pytest.raises(_TestIsSkip):
        test.is_skip()


def test_is_skip_glx(mocker):
    """test.gleantest.GleanTest.is_skip: Does not skip when platform is glx."""
    opts = mocker.patch('framework.test.gleantest.options.OPTIONS',
                        new_callable=Options)
    opts.env['PIGLIT_PLATFORM'] = 'glx'

    test = GleanTest('foo')
    test.is_skip()


def test_is_skip_glx_egl(mocker):
    """test.gleantest.GleanTest.is_skip: Does not skip when platform is
    mixed_glx_egl."""
    opts = mocker.patch('framework.test.gleantest.options.OPTIONS',
                        new_callable=Options)
    opts.env['PIGLIT_PLATFORM'] = 'mixed_glx_egl'

    test = GleanTest('foo')
    test.is_skip()


def test_crash():
    """test.gleantest.GleanTest.interpret_result: Crashes are set to crash"""
    test = GleanTest('foo')
    test.result.returncode = -5
    test.interpret_result()

    assert test.result.result is status.CRASH
