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

from __future__ import print_function
import os
from nose.plugins.skip import SkipTest
from framework.gleantest import GleanTest


def test_initialize_gleantest():
    """ Test that GleanTest initilizes """
    test = GleanTest('name')
    assert test


def test_globalParams_assignment():
    """ Test to ensure that GleanTest.globalParams are correctly assigned

    Specifically this tests for a bug where globalParams only affected
    instances of GleanTest created after globalParams were set, so changing the
    globalParams value had unexpected results.

    If this test passes the GleanTest.command attributes will be the same in
    the instance created before the globalParams assignment and the one created
    after. A failure means the that globalParams are not being added to tests
    initialized before it is set.

    """
    test1 = GleanTest('basic')
    GleanTest.globalParams = ['--quick']
    test2 = GleanTest('basic')
    assert test1.command == test2.command


def test_bad_returncode():
    """ If glean doesn't return zero it should be a fail

    Currently clean returns 127 if piglit can't find it's libs (LD_LIBRARY_PATH
    isn't set properly), and then marks such tests as pass, when they obviously
    are not.

    """
    if not os.path.exists('bin'):
        raise SkipTest("This test requires a working, built version of piglit")

    # Clearing the environment should remove the piglit/lib from
    # LD_LIBRARY_PATH
    os.environ = {}

    test = GleanTest('basic')
    result = test.run()
    print("result: {result}\nreturncode: {returncode}".format(**result))
    assert result['result'] == 'fail', "Result should have been fail"
