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

import nose.tools as nt

import framework.tests.utils as utils
from framework.log import LogManager
from framework.dmesg import DummyDmesg
from framework.test import PiglitTest, Test


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
def test_initialize_piglittest():
    """ Test that PiglitTest initializes correctly """
    PiglitTest('/bin/true')


def test_run_return_early():
    """ Test.run() exits early when Test._run_command() has exception """
    def helper():
        raise AssertionError("The test didn't return early")

    # Of course, this won't work if you actually have a foobarcommand in your
    # path...
    test = TestTest(['foobarcommand'])
    test.test_interpret_result = helper
    test.run()


def test_timeout():
    """ Test that Test.timeout works correctly """
    utils.binary_check('sleep')

    def helper():
        if (test.result['returncode'] == 0):
            test.result['result'] = "pass"

    test = TestTest("sleep 60")
    test.test_interpret_result = helper
    test.timeout = 1
    test.run()
    assert test.result['result'] == 'timeout'


def test_timeout_pass():
    """ Test that the correct result is returned if a test does not timeout """
    utils.binary_check('true')

    def helper():
        if (test.result['returncode'] == 0):
            test.result['result'] = "pass"

    test = TestTest("true")
    test.test_interpret_result = helper
    test.timeout = 1
    test.run()
    assert test.result['result'] == 'pass'


def test_piglittest_interpret_result():
    """ PiglitTest.interpret_result() works no subtests """
    test = PiglitTest('foo')
    test.result['out'] = 'PIGLIT: {"result": "pass"}\n'
    test.interpret_result()
    assert test.result['result'] == 'pass'


def test_piglittest_interpret_result_subtest():
    """ PiglitTest.interpret_result() works with subtests """
    test = PiglitTest('foo')
    test.result['out'] = ('PIGLIT: {"result": "pass"}\n'
                          'PIGLIT: {"subtest": {"subtest": "pass"}}\n')
    test.interpret_result()
    assert test.result['subtest']['subtest'] == 'pass'


def test_piglitest_no_clobber():
    """ PiglitTest.interpret_result() does not clobber subtest entires """
    test = PiglitTest(['a', 'command'])
    test.result['out'] = (
        'PIGLIT: {"result": "pass"}\n'
        'PIGLIT: {"subtest": {"test1": "pass"}}\n'
        'PIGLIT: {"subtest": {"test2": "pass"}}\n'
    )
    test.interpret_result()

    nt.assert_dict_equal(test.result['subtest'],
                         {'test1': 'pass', 'test2': 'pass'})


def test_log_expanding_running():
    """Test.execute(): When a test fails the default value is valid.

    This exercises a bug where the default value is a status.Status object,
    which causes a failure in the logger. Because of the way that python
    handles thread exceptions the thread dies, and the number is left in the
    running tests.

    """
    class _Test(Test):
        """Class for testing execute() default value."""
        def run(self):
            pass

        def interpret_result(self):
            raise AssertionError('run did not return')

    manager = LogManager('quiet', 5)

    test = _Test('foo')
    test.execute('foo', manager.get(), DummyDmesg())
    nt.assert_list_equal(manager._state['running'], [])
