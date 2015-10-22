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
import tempfile
import textwrap
import os

import mock
import nose.tools as nt
from nose.plugins.attrib import attr

try:
    import psutil
except ImportError:
    pass

from . import utils
from .status_tests import PROBLEMS, STATUSES
from framework.test.base import (
    Test,
    TestRunError,
    ValgrindMixin,
    WindowResizeMixin,
)
from framework.options import _Options as Options

# pylint: disable=invalid-name

# Helpers
class TestTest(Test):
    """ A class for testing that implements a dummy interpret_result

    interpret_result() can ve overwritten by setting the
    self.test_interpret_result name

    """
    test_interpret_result = lambda: None

    def interpret_result(self):
        self.test_interpret_result()


class TimeoutTest(Test):
    def interpret_result(self):
        super(TimeoutTest, self).interpret_result()


# Tests
def test_run_return_early():
    """ Test.run() exits early when Test._run_command() has exception """
    def helper():
        raise utils.TestFailure("The test didn't return early")

    # Of course, this won't work if you actually have a foobarcommand in your
    # path...
    test = TestTest(['foobaroinkboink_zing'])
    test.test_interpret_result = helper
    test.run()


@attr('slow')
@nt.timed(6)
def test_timeout_kill_children():
    """test.base.Test: kill children if terminate fails

    This creates a process that forks multiple times, and then checks that the
    children have been killed.

    This test could leave processes running if it fails.

    """
    utils.module_check('subprocess32')
    utils.module_check('psutil')

    import subprocess32  # pylint: disable=import-error

    class PopenProxy(object):
        """An object that proxies Popen, and saves the Popen instance as an
        attribute.

        This is useful for testing the Popen instance.

        """
        def __init__(self):
            self.popen = None

        def __call__(self, *args, **kwargs):
            self.popen = subprocess32.Popen(*args, **kwargs)

            # if commuincate cis called successfully then the proc will be
            # reset to None, whic will make the test fail.
            self.popen.communicate = mock.Mock(return_value=('out', 'err'))

            return self.popen

    with tempfile.NamedTemporaryFile() as f:
        # Create a file that will be executed as a python script
        # Create a process with two subproccesses (not threads) that will run
        # for a long time.
        f.write(textwrap.dedent("""\
            import time
            from multiprocessing import Process

            def p():
                for _ in range(100):
                    time.sleep(1)

            a = Process(target=p)
            b = Process(target=p)
            a.start()
            b.start()
            a.join()
            b.join()
        """))
        f.seek(0)  # we'll need to read the file back

        # Create an object that will return a popen object, but also store it
        # so we can access it later
        proxy = PopenProxy()

        test = TimeoutTest(['python', f.name])
        test.timeout = 1

        # mock out subprocess.Popen with our proxy object
        with mock.patch('framework.test.base.subprocess') as mock_subp:
            mock_subp.Popen = proxy
            mock_subp.TimeoutExpired = subprocess32.TimeoutExpired
            test.run()

        # Check to see if the Popen has children, even after it should have
        # recieved a TimeoutExpired.
        proc = psutil.Process(os.getsid(proxy.popen.pid))
        children = proc.children(recursive=True)

        if children:
            # If there are still running children attempt to clean them up,
            # starting with the final generation and working back to the first
            for child in reversed(children):
                child.kill()

            raise utils.TestFailure(
                'Test process had children when it should not')


@attr('slow')
@nt.timed(6)
def test_timeout():
    """test.base.Test: Stops running test after timeout expires

    This is a little bit of extra time here, but without a sleep of 60 seconds
    if the test runs 5 seconds it's run too long

    """
    utils.module_check('subprocess32')
    utils.binary_check('sleep', 1)

    test = TimeoutTest(['sleep', '60'])
    test.timeout = 1
    test.run()


@attr('slow')
@nt.timed(6)
def test_timeout_timeout():
    """test.base.Test: Sets status to 'timeout' when timeout exceeded"""
    utils.module_check('subprocess32')
    utils.binary_check('sleep', 1)

    test = TimeoutTest(['sleep', '60'])
    test.timeout = 1
    test.run()
    nt.eq_(test.result.result, 'timeout')


@nt.timed(2)
def test_timeout_pass():
    """test.base.Test: Doesn't change status when timeout not exceeded
    """
    utils.module_check('subprocess32')
    utils.binary_check('true')

    test = TimeoutTest(['true'])
    test.timeout = 1
    test.result.result = 'pass'
    test.run()
    nt.eq_(test.result.result, 'pass')


def test_WindowResizeMixin_rerun():
    """test.base.WindowResizeMixin: runs multiple when spurious resize detected
    """
    # Because of Python's inheritance order we need the mixin here.
    class Mixin(object):
        def __init__(self, *args, **kwargs):
            super(Mixin, self).__init__(*args, **kwargs)
            self.__return_spurious = True

        def _run_command(self):
            self.result.returncode = None

            # IF this is run only once we'll have "got spurious window resize"
            # in result.out, if it runs multiple times we'll get 'all good'
            if self.__return_spurious:
                self.result.out = "Got spurious window resize"
                self.__return_spurious = False
            else:
                self.result.out = 'all good'

    class Test_(WindowResizeMixin, Mixin, Test):
        def interpret_result(self):
            pass

    test = Test_(['foo'])
    test.run()
    nt.assert_equal(test.result.out, 'all good')


def test_run_command_early():
    """test.base.Test.run(): returns early if there is an error in _run_command()
    """
    class Test_(Test):
        def interpret_result(self):
            raise utils.TestFailure("The test didn't return early")

        def _run_command(self):
            raise TestRunError('an error', 'skip')

    # Of course, if there is an executable 'foobarboinkoink' in your path this
    # test will fail. It seems pretty unlikely that you would
    test = Test_(['foobarboinkoink'])
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


@mock.patch('framework.test.base.options.OPTIONS', new_callable=Options)
def test_ValgrindMixin_command(mock_opts):
    """test.base.ValgrindMixin.command: overrides self.command"""
    class _Test(ValgrindMixin, utils.Test):
        pass
    mock_opts.valgrind = True

    test = _Test(['foo'])
    nt.eq_(test.command, ['valgrind', '--quiet', '--error-exitcode=1',
                          '--tool=memcheck', 'foo'])


class TestValgrindMixinRun(object):
    @classmethod
    def setup_class(cls):
        class _NoRunTest(utils.Test):
            def run(self):
                self.interpret_result()

        class _Test(ValgrindMixin, _NoRunTest):
            pass

        cls.test = _Test

    @utils.nose_generator
    def test_bad_valgrind_true(self):
        """Test non-pass status when options.OPTIONS.valgrind is True."""
        def test(status, expected):
            test = self.test(['foo'])
            test.result.result = status

            with mock.patch('framework.test.base.options.OPTIONS',
                    new_callable=Options) as mock_opts:
                mock_opts.valgrind = True
                test.run()

            nt.eq_(test.result.result, expected)

        desc = ('test.base.ValgrindMixin.run: '
                'when status is "{}" it is changed to "{}"')

        for status in PROBLEMS:
            test.description = desc.format(status, 'skip')
            yield test, status, 'skip'

    @utils.nose_generator
    def test_valgrind_false(self):
        """Test non-pass status when options.OPTIONS.valgrind is False."""
        def test(status):
            test = self.test(['foo'])
            test.result.result = status

            with mock.patch('framework.test.base.options.OPTIONS',
                    new_callable=Options) as mock_opts:
                mock_opts.valgrind = False
                test.run()

            nt.eq_(test.result.result, status)

        desc = ('test.base.ValgrindMixin.run: when status is "{}" '
                'it is not changed when not running valgrind.')

        for status in STATUSES:
            test.description = desc.format(status)
            yield test, status

    @mock.patch('framework.test.base.options.OPTIONS', new_callable=Options)
    def test_pass(self, mock_opts):
        """test.base.ValgrindMixin.run: when test is 'pass' and returncode is '0' result is pass
        """
        test = self.test(['foo'])
        mock_opts.valgrind = True
        test.result.result = 'pass'
        test.result.returncode = 0
        test.run()
        nt.eq_(test.result.result, 'pass')

    @mock.patch('framework.test.base.options.OPTIONS', new_callable=Options)
    def test_fallthrough(self, mock_opts):
        """test.base.ValgrindMixin.run: when a test is 'pass' but returncode is not 0 it's 'fail'
        """
        test = self.test(['foo'])
        mock_opts.valgrind = True
        test.result.result = 'pass'
        test.result.returncode = 1
        test.run()
        nt.eq_(test.result.result, 'fail')


def test_interpret_result_greater_zero():
    """test.base.Test.interpret_result: A test with status > 0 is fail"""
    class _Test(Test):
        def interpret_result(self):
            super(_Test, self).interpret_result()

    test = _Test(['foobar'])
    test.result.returncode = 1
    test.result.out = 'this is some\nstdout'
    test.result.err = 'this is some\nerrors'
    test.interpret_result()

    nt.eq_(test.result.result, 'fail')
