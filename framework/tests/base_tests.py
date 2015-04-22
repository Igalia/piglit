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

import framework.tests.utils as utils
from framework.test.base import Test, WindowResizeMixin


# Helpers
class TestTest(Test):
    """ A class for testing that implements a dummy interpret_result

    interpret_result() can ve overwritten by setting the
    self.test_interpret_result name

    """
    test_interpret_result = lambda: None

    def interpret_result(self):
        self.test_interpret_result()


# Tests
def test_timeout():
    """test.base.Test.run(): kills tests that exceed timeout when set"""
    utils.binary_check('sleep')

    def helper():
        if (test.result['returncode'] == 0):
            test.result['result'] = "pass"

    test = TestTest(['sleep', '60'])
    test.test_interpret_result = helper
    test.timeout = 1
    test.run()
    assert test.result['result'] == 'timeout'


def test_timeout_pass():
    """test.base.Test.run(): Result is returned when timeout is set but not exceeded
    """
    utils.binary_check('true')

    def helper():
        if (test.result['returncode'] == 0):
            test.result['result'] = "pass"

    test = TestTest(['true'])
    test.test_interpret_result = helper
    test.timeout = 1
    test.run()
    assert test.result['result'] == 'pass'


def test_WindowResizeMixin_rerun():
    """test.base.WindowResizeMixin: runs multiple when spurious resize detected
    """
    # Because of Python's inheritance order we need the mixin here.
    class Mixin(object):
        def __init__(self, *args, **kwargs):
            super(Mixin, self).__init__(*args, **kwargs)
            self.__return_spurious = True

        def _run_command(self):
            self.result['returncode'] = None

            # IF this is run only once we'll have "got spurious window resize"
            # in result['out'], if it runs multiple times we'll get 'all good'
            if self.__return_spurious:
                self.result['out'] = "Got spurious window resize"
                self.__return_spurious = False
            else:
                self.result['out'] = 'all good'

    class Test_(WindowResizeMixin, Mixin, Test):
        def interpret_result(self):
            pass

    test = Test_(['foo'])
    test.run()
    nt.assert_equal(test.result['out'], 'all good')


def test_run_command_early():
    """test.base.Test.run(): returns early if there is an error in _run_command()
    """
    class Test_(Test):
        def interpret_result(self):
            raise AssertionError('Test.run() did not return early')

        def _run_command(self):
            return True

    test = Test_(['foo'])
    test.run()


@nt.raises(AssertionError)
def test_no_string():
    """test.base.Test.__init__: Asserts if it is passed a string instead of a list"""
    TestTest('foo')


def test_mutation():
    """test.base.Test.command: does not mutate the value it was provided

    There is a very subtle bug in all.py that causes the key values to be
    changed before they are assigned in some cases. This is because the right
    side of an assignment is evalated before the left side, so

    >>> profile = {}
    >>> args = ['a', 'b']
    >>> profile[' '.join(args)] = PiglitGLTest(args)
    >>> profile.keys()
    ['bin/a b']

    """
    class _Test(TestTest):
        def __init__(self, *args, **kwargs):
            super(_Test, self).__init__(*args, **kwargs)
            self._command[0] = 'bin/' + self._command[0]

    args = ['a', 'b']
    _Test(args)

    nt.assert_list_equal(args, ['a', 'b'])
