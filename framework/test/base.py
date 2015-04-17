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

from framework import exceptions
from framework.core import Options
from framework.results import TestResult


__all__ = [
    'Test',
    'WindowResizeMixin',
]


class TestIsSkip(exceptions.PiglitException):
    """Exception raised in is_skip() if the test is a skip."""
    pass


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
    OPTS = Options()
    __metaclass__ = abc.ABCMeta
    __slots__ = ['run_concurrent', 'env', 'result', 'cwd', '_command',
                 '_test_hook_execute_run', '__proc_timeout']
    timeout = 0

    def __init__(self, command, run_concurrent=False):
        assert isinstance(command, list), command

        self.run_concurrent = run_concurrent
        self._command = copy.copy(command)
        self.env = {}
        self.result = TestResult({'result': 'fail'})
        self.cwd = None
        self.__proc_timeout = None

        # This is a hook for doing some testing on execute right before
        # self.run is called.
        self._test_hook_execute_run = lambda: None

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
        if self.OPTS.execute:
            try:
                time_start = time.time()
                dmesg.update_dmesg()
                self._test_hook_execute_run()
                self.run()
                self.result['time'] = time.time() - time_start
                self.result = dmesg.update_result(self.result)
            # This is a rare case where a bare exception is okay, since we're
            # using it to log exceptions
            except:
                exception = sys.exc_info()
                self.result['result'] = 'fail'
                self.result['exception'] = "{}{}".format(*exception[:2])
                self.result['traceback'] = "".join(
                    traceback.format_tb(exception[2]))

            log.log(self.result['result'])
        else:
            log.log('dry-run')

    @property
    def command(self):
        assert self._command
        if self.OPTS.valgrind:
            return ['valgrind', '--quiet', '--error-exitcode=1',
                    '--tool=memcheck'] + self._command
        return self._command

    @abc.abstractmethod
    def interpret_result(self):
        """ Convert the raw output of the test into a form piglit understands
        """

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
        self.result['command'] = ' '.join(self.command)
        self.result['environment'] = " ".join(
            '{0}="{1}"'.format(k, v) for k, v in itertools.chain(
                self.OPTS.env.iteritems(), self.env.iteritems()))

        try:
            self.is_skip()
        except TestIsSkip as e:
            self.result['result'] = 'skip'
            self.result['out'] = e.message
            self.result['err'] = ""
            self.result['returncode'] = None
            return

        run_error = self._run_command()
        if run_error:
            # If there was an error self.result should already have been set,
            # return early
            return

        self.interpret_result()

        if self.result['returncode'] < 0:
            # check if the process was terminated by the timeout
            if self.timeout > 0 and self.__proc_timeout.join() > 0:
                self.result['result'] = 'timeout'
            else:
                self.result['result'] = 'crash'
        elif self.result['returncode'] != 0 and self.result['result'] == 'pass':
            self.result['result'] = 'warn'

        if self.OPTS.valgrind:
            # If the underlying test failed, simply report
            # 'skip' for this valgrind test.
            if self.result['result'] != 'pass':
                self.result['result'] = 'skip'
            elif self.result['returncode'] == 0:
                # Test passes and is valgrind clean.
                self.result['result'] = 'pass'
            else:
                # Test passed but has valgrind errors.
                self.result['result'] = 'fail'

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
        # if there is an error in run command this will be set to True, if this
        # is True then the run test method will return early. If this is set to
        # True then self.result should be properly filled out
        error = False

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
                                          self.OPTS.env.iteritems(),
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

            out, err = proc.communicate()
            returncode = proc.returncode
        except OSError as e:
            # Different sets of tests get built under
            # different build configurations.  If
            # a developer chooses to not build a test,
            # Piglit should not report that test as having
            # failed.
            if e.errno == errno.ENOENT:
                self.result['result'] = 'skip'
                out = "Test executable not found.\n"
                err = ""
                returncode = None
                error = True
            else:
                raise e

        # proc.communicate() returns 8-bit strings, but we need
        # unicode strings.  In Python 2.x, this is because we
        # will eventually be serializing the strings as JSON,
        # and the JSON library expects unicode.  In Python 3.x,
        # this is because all string operations require
        # unicode.  So translate the strings into unicode,
        # assuming they are using UTF-8 encoding.
        #
        # If the subprocess output wasn't properly UTF-8
        # encoded, we don't want to raise an exception, so
        # translate the strings using 'replace' mode, which
        # replaces erroneous charcters with the Unicode
        # "replacement character" (a white question mark inside
        # a black diamond).
        self.result['out'] = out.decode('utf-8', 'replace')
        self.result['err'] = err.decode('utf-8', 'replace')
        self.result['returncode'] = returncode

        return error

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
            err = super(WindowResizeMixin, self)._run_command()
            if err:
                return err
            elif "Got spurious window resize" not in self.result['out']:
                return False

        # If we reach this point then there has been no error, but spurious
        # resize was detected more than 5 times. Set the result to fail, and
        # add a message about why, and return True so that the test will exit
        # early
        self.result['result'] = 'fail'
        self.result['err'] = unicode()
        self.result['out'] = unicode('Got spurious resize more than 5 times')
        self.result['returncode'] = None
        return True
