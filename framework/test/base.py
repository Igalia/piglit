#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

""" Module provides a base class for Tests """

from __future__ import print_function, absolute_import
import errno
import os
import subprocess
import time
import sys
import traceback
from datetime import datetime
import threading
import signal
import itertools
import abc
import copy

from framework import exceptions, options
from framework.results import TestResult


__all__ = [
    'Test',
    'TestIsSkip',
    'TestRunError',
    'ValgrindMixin',
    'WindowResizeMixin',
    'is_crash_returncode',
]


class TestIsSkip(exceptions.PiglitException):
    """Exception raised in is_skip() if the test is a skip."""
    pass


class TestRunError(exceptions.PiglitException):
    """Exception raised if the test fails to run."""
    def __init__(self, message, status):
        super(TestRunError, self).__init__(message)
        self.status = status


class ProcessTimeout(threading.Thread):
    """ Timeout class for test processes

    This class is for terminating tests that run for longer than a certain
    amount of time. Create an instance by passing it a timeout in seconds
    and the Popen object that is running your test. Then call the start
    method (inherited from Thread) to start the timer. The Popen object is
    killed if the timeout is reached and it has not completed. Wait for the
    outcome by calling the join() method from the parent.

    """

    def __init__(self, timeout, proc):
        threading.Thread.__init__(self)
        self.proc = proc
        self.timeout = timeout
        self.status = 0

    def run(self):
        start_time = datetime.now()
        delta = 0

        # poll() returns the returncode attribute, which is either the return
        # code of the child process (which could be zero), or None if the
        # process has not yet terminated.

        while delta < self.timeout and self.proc.poll() is None:
            time.sleep(1)
            delta = (datetime.now() - start_time).total_seconds()

        # if the test is not finished after timeout, first try to terminate it
        # and if that fails, send SIGKILL to all processes in the test's
        # process group

        if self.proc.poll() is None:
            self.status = 1
            self.proc.terminate()
            time.sleep(5)

        if self.proc.poll() is None:
            self.status = 2
            if hasattr(os, 'killpg'):
                os.killpg(self.proc.pid, signal.SIGKILL)
            self.proc.wait()

    def join(self):
        threading.Thread.join(self)
        return self.status


def is_crash_returncode(returncode):
    """Determine whether the given process return code correspond to a
    crash.
    """
    if sys.platform == 'win32':
        # On Windows:
        # - For uncaught exceptions the process terminates with the exception
        # code, which is usually negative
        # - MSVCRT's abort() terminates process with exit code 3
        return returncode < 0 or returncode == 3
    else:
        return returncode < 0


class Test(object):
    """ Abstract base class for Test classes

    This class provides the framework for running tests, with several methods
    and properties that can be overwritten to produce a specialized class for
    running test suites other than piglit.

    It provides two methods for running tests, excecute and run.
    execute() provides lots of features, and is invoced when running piglit
    from the command line, run() is a more basic method for running the test,
    and is called internally by execute(), but is can be useful outside of it.

    Arguments:
    command -- a value to be passed to subprocess.Popen

    Keyword Arguments:
    run_concurrent -- If True the test is thread safe. Default: False

    """
    __metaclass__ = abc.ABCMeta
    __slots__ = ['run_concurrent', 'env', 'result', 'cwd', '_command',
                 '__proc_timeout']
    timeout = 0

    def __init__(self, command, run_concurrent=False):
        assert isinstance(command, list), command

        self.run_concurrent = run_concurrent
        self._command = copy.copy(command)
        self.env = {}
        self.result = TestResult()
        self.cwd = None
        self.__proc_timeout = None

    def execute(self, path, log, dmesg):
        """ Run a test

        Run a test, but with features. This times the test, uses dmesg checking
        (if requested), and runs the logger.

        Arguments:
        path -- the name of the test
        log -- a log.Log instance
        dmesg -- a dmesg.BaseDmesg derived class

        """
        log.start(path)
        # Run the test
        if options.OPTIONS.execute:
            try:
                self.result.time.start = time.time()
                dmesg.update_dmesg()
                self.run()
                self.result.time.end = time.time()
                self.result = dmesg.update_result(self.result)
            # This is a rare case where a bare exception is okay, since we're
            # using it to log exceptions
            except:
                exc_type, exc_value, exc_traceback = sys.exc_info()
                traceback.print_exc(file=sys.stderr)
                self.result.result = 'fail'
                self.result.exception = "{}{}".format(exc_type, exc_value)
                self.result.traceback = "".join(
                    traceback.format_tb(exc_traceback))

            log.log(self.result.result)
        else:
            log.log('dry-run')

    @property
    def command(self):
        assert self._command
        return self._command

    @abc.abstractmethod
    def interpret_result(self):
        """Convert the raw output of the test into a form piglit understands.
        """
        if is_crash_returncode(self.result.returncode):
            # check if the process was terminated by the timeout
            if self.timeout > 0 and self.__proc_timeout.join() > 0:
                self.result.result = 'timeout'
            else:
                self.result.result = 'crash'
        elif self.result.returncode != 0:
            if self.result.result == 'pass':
                self.result.result = 'warn'
            else:
                self.result.result = 'fail'

    def run(self):
        """
        Run a test.  The return value will be a dictionary with keys
        including 'result', 'info', 'returncode' and 'command'.
        * For 'result', the value may be one of 'pass', 'fail', 'skip',
          'crash', or 'warn'.
        * For 'info', the value will include stderr/out text.
        * For 'returncode', the value will be the numeric exit code/value.
        * For 'command', the value will be command line program and arguments.
        """
        self.result.command = ' '.join(self.command)
        self.result.environment = " ".join(
            '{0}="{1}"'.format(k, v) for k, v in itertools.chain(
                options.OPTIONS.env.iteritems(), self.env.iteritems()))

        try:
            self.is_skip()
        except TestIsSkip as e:
            self.result.result = 'skip'
            self.result.out = unicode(e)
            self.result.returncode = None
            return

        try:
            self._run_command()
        except TestRunError as e:
            self.result.result = unicode(e.status)
            self.result.out = unicode(e)
            self.result.returncode = None
            return

        self.interpret_result()

    def is_skip(self):
        """ Application specific check for skip

        If this function returns a truthy value then the current test will be
        skipped. The base version will always return False

        """
        pass

    def __set_process_group(self):
        if hasattr(os, 'setpgrp'):
            os.setpgrp()

    def _run_command(self):
        """ Run the test command and get the result

        This method sets environment options, then runs the executable. If the
        executable isn't found it sets the result to skip.

        """
        # Setup the environment for the test. Environment variables are taken
        # from the following sources, listed in order of increasing precedence:
        #
        #   1. This process's current environment.
        #   2. Global test options. (Some of these are command line options to
        #      Piglit's runner script).
        #   3. Per-test environment variables set in all.py.
        #
        # Piglit chooses this order because Unix tradition dictates that
        # command line options (2) override environment variables (1); and
        # Piglit considers environment variables set in all.py (3) to be test
        # requirements.
        #
        fullenv = dict()
        for key, value in itertools.chain(os.environ.iteritems(),
                                          options.OPTIONS.env.iteritems(),
                                          self.env.iteritems()):
            fullenv[key] = str(value)

        # preexec_fn is not supported on Windows platforms
        if sys.platform == 'win32':
            preexec_fn = None
        else:
            preexec_fn = self.__set_process_group

        try:
            proc = subprocess.Popen(self.command,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    cwd=self.cwd,
                                    env=fullenv,
                                    universal_newlines=True,
                                    preexec_fn=preexec_fn)

            # create a ProcessTimeout object to watch out for test hang if the
            # process is still going after the timeout, then it will be killed
            # forcing the communicate function (which is a blocking call) to
            # return
            if self.timeout > 0:
                self.__proc_timeout = ProcessTimeout(self.timeout, proc)
                self.__proc_timeout.start()

            self.result.pid = proc.pid

            out, err = proc.communicate()
            returncode = proc.returncode
        except OSError as e:
            # Different sets of tests get built under different build
            # configurations.  If a developer chooses to not build a test,
            # Piglit should not report that test as having failed.
            if e.errno == errno.ENOENT:
                raise TestRunError("Test executable not found.\n", 'skip')
            else:
                raise e

        # The setter handles the bytes/unicode conversion
        self.result.out = out
        self.result.err = err
        self.result.returncode = returncode

    def __eq__(self, other):
        return self.command == other.command

    def __ne__(self, other):
        return not self == other


class WindowResizeMixin(object):
    """ Mixin class that deals with spurious window resizes

    On gnome (and possible other DE's) the window manager may decide to resize
    a window. This causes the test to fail even though otherwise would not.
    This Mixin overides the _run_command method to run the test 5 times, each
    time searching for the string 'Got suprious window resize' in the output,
    if it fails to find it it will break the loop and continue.

    see: https://bugzilla.gnome.org/show_bug.cgi?id=680214

    """
    def _run_command(self):
        """Run a test up 5 times when window resize is detected.

        Rerun the command up to 5 times if the window size changes, if it
        changes 6 times mark the test as fail and return True, which will cause
        Test.run() to return early.

        """
        for _ in xrange(5):
            super(WindowResizeMixin, self)._run_command()
            if "Got spurious window resize" not in self.result.out:
                return

        # If we reach this point then there has been no error, but spurious
        # resize was detected more than 5 times. Set the result to fail
        raise TestRunError('Got spurious resize more than 5 times', 'fail')


class ValgrindMixin(object):
    """Mixin class that adds support for running tests through valgrind.

    This mixin allows a class to run with the --valgrind option.

    """
    @Test.command.getter
    def command(self):
        command = super(ValgrindMixin, self).command
        if options.OPTIONS.valgrind:
            return ['valgrind', '--quiet', '--error-exitcode=1',
                    '--tool=memcheck'] + command
        else:
            return command

    def interpret_result(self):
        """Set the status to the valgrind status.

        It is important that the valgrind interpret_results code is run last,
        since it depends on the statuses already set and passed to it,
        including the Test.interpret_result() method. To this end it executes
        super().interpret_result(), then calls it's own result.

        """
        super(ValgrindMixin, self).interpret_result()

        if options.OPTIONS.valgrind:
            # If the underlying test failed, simply report
            # 'skip' for this valgrind test.
            if self.result.result != 'pass':
                self.result.result = 'skip'
            elif self.result.returncode == 0:
                # Test passes and is valgrind clean.
                self.result.result = 'pass'
            else:
                # Test passed but has valgrind errors.
                self.result.result = 'fail'
