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

from framework.test import GleanTest


def test_initialize_gleantest():
    """ Test that GleanTest initilizes """
    test = GleanTest('name')
    assert test


def test_GLOBAL_PARAMS_assignment():
    """ Test to ensure that GleanTest.GLOBAL_PARAMS are correctly assigned

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


def test_bad_returncode():
    """ Result is 'Fail' when returncode is not 0

    Currently clean returns 127 if piglit can't find it's libs (LD_LIBRARY_PATH
    isn't set properly), and then marks such tests as pass, when they obviously
    are not.

    """
    test = GleanTest('basic')
    test.result['returncode'] = 1
    test.interpret_result()
    assert test.result['result'] == 'fail', "Result should have been fail"
