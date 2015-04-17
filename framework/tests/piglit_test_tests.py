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

""" Tests for the exectest module """

from __future__ import print_function, absolute_import

import nose.tools as nt

from framework.tests import utils
from framework.test.base import TestIsSkip
from framework.test.piglit_test import (PiglitBaseTest, PiglitGLTest,
                                        PiglitCLTest)


@utils.no_error
def test_initialize_piglitgltest():
    """test.piglit_test.PiglitGLTest: Class initializes"""
    PiglitGLTest(['/bin/true'])


@utils.no_error
def test_initialize_piglitcltest():
    """test.piglit_test.PiglitCLTest: Class initializes"""
    PiglitCLTest(['/bin/true'])


def test_piglittest_interpret_result():
    """test.piglit_test.PiglitBaseTest.interpret_result(): works no subtests"""
    test = PiglitBaseTest(['foo'])
    test.result['out'] = 'PIGLIT: {"result": "pass"}\n'
    test.interpret_result()
    assert test.result['result'] == 'pass'


def test_piglittest_interpret_result_subtest():
    """test.piglit_test.PiglitBaseTest.interpret_result(): works with subtests"""
    test = PiglitBaseTest(['foo'])
    test.result['out'] = ('PIGLIT: {"result": "pass"}\n'
                          'PIGLIT: {"subtest": {"subtest": "pass"}}\n')
    test.interpret_result()
    assert test.result['subtest']['subtest'] == 'pass'


def test_piglitest_no_clobber():
    """test.piglit_test.PiglitBaseTest.interpret_result(): does not clobber subtest entires"""
    test = PiglitBaseTest(['a', 'command'])
    test.result['out'] = (
        'PIGLIT: {"result": "pass"}\n'
        'PIGLIT: {"subtest": {"test1": "pass"}}\n'
        'PIGLIT: {"subtest": {"test2": "pass"}}\n'
    )
    test.interpret_result()

    nt.assert_dict_equal(test.result['subtest'],
                         {'test1': 'pass', 'test2': 'pass'})


def test_piglittest_command_getter_serial():
    """test.piglit_test.PiglitGLTest.command: adds -auto to serial tests"""
    test = PiglitGLTest(['foo'])
    nt.assert_in('-auto', test.command)


def test_piglittest_command_getter_concurrent():
    """test.piglit_test.PiglitGLTest.command: adds -fbo and -auto to concurrent tests"""
    test = PiglitGLTest(['foo'], run_concurrent=True)
    nt.assert_in('-auto', test.command)
    nt.assert_in('-fbo', test.command)


def test_PiglitGLTest_include_and_exclude():
    """test.piglit_test.PiglitGLTest.is_skip(): raises if include and exclude are given."""
    with nt.assert_raises(AssertionError):
        PiglitGLTest(['foo'],
                     require_platforms=['glx'],
                     exclude_platforms=['gbm'])


@utils.not_raises(TestIsSkip)
def test_PiglitGLTest_platform_in_require():
    """test.piglit_test.PiglitGLTest.is_skip(): does not skip if platform is in require_platforms"""
    PiglitGLTest.OPTS.env['PIGLIT_PLATFORM'] = 'glx'
    test = PiglitGLTest(['foo'], require_platforms=['glx'])
    test.is_skip()


@nt.raises(TestIsSkip)
def test_PiglitGLTest_platform_not_in_require():
    """test.piglit_test.PiglitGLTest.is_skip(): skips if platform is not in require_platforms"""
    PiglitGLTest.OPTS.env['PIGLIT_PLATFORM'] = 'gbm'
    test = PiglitGLTest(['foo'], require_platforms=['glx'])
    test.is_skip()


@nt.raises(TestIsSkip)
def test_PiglitGLTest_platform_in_exclude():
    """test.piglit_test.PiglitGLTest.is_skip(): skips if platform is in exclude_platforms"""
    PiglitGLTest.OPTS.env['PIGLIT_PLATFORM'] = 'glx'
    test = PiglitGLTest(['foo'], exclude_platforms=['glx'])
    test.is_skip()


@utils.not_raises(TestIsSkip)
def test_PiglitGLTest_platform_not_in_exclude():
    """test.piglit_test.PiglitGLTest.is_skip(): does not skip if platform is in exclude_platforms"""
    PiglitGLTest.OPTS.env['PIGLIT_PLATFORM'] = 'gbm'
    test = PiglitGLTest(['foo'], exclude_platforms=['glx'])
    test.is_skip()
