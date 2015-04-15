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

from __future__ import print_function, absolute_import

import nose.tools as nt

from framework.test import GleanTest
from framework.tests import utils


@utils.no_error
def test_initialize_gleantest():
    """test.gleantest.GleanTest: class initializes correctly"""
    GleanTest('name')


def test_GLOBAL_PARAMS_assignment():
    """test.gleantest.GleanTest: GLOBAL_PARAMS apply to instances created
    after GLABL_PARAMS is set

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
    nt.assert_list_equal(test1.command, test2.command)


def test_bad_returncode():
    """test.gleantest.GleanTest: If returncode is 0 the result is 'fail'

    Currently clean returns 127 if piglit can't find it's libs (LD_LIBRARY_PATH
    isn't set properly), and then marks such tests as pass, when they obviously
    are not.

    """
    test = GleanTest('basic')
    test.result['returncode'] = 1
    test.interpret_result()
    nt.assert_equal(test.result['result'], 'fail',
                    msg="Result should have been fail")
