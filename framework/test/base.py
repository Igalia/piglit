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

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import errno
import os
import time
import sys
import traceback
import itertools
import abc
import copy
import signal
import warnings

import six
from six.moves import range

from framework import exceptions
from framework import status
from framework.options import OPTIONS
from framework.results import TestResult

# We're doing some special crazy here to make timeouts work on python 2. pylint
# is going to complain a lot
# pylint: disable=wrong-import-position,wrong-import-order
if six.PY2:
    try:
        # subprocess32 only supports *nix systems, this is important because
        # "start_new_session" is not a valid argument on windows

        import subprocess32 as subprocess
        _EXTRA_POPEN_ARGS = {'start_new_session': True}
    except ImportError:
        # If there is no timeout support, fake it. Add a TimeoutExpired
        # exception and a Popen that accepts a timeout parameter (and ignores
        # it), then shadow the actual Popen with this wrapper.

        import subprocess

        class TimeoutExpired(Exception):
            pass

        class Popen(subprocess.Popen):
            """Sublcass of Popen that accepts and ignores a timeout argument."""
            def communicate(self, *args, **kwargs):
                if 'timeout' in kwargs:
                    del kwargs['timeout']
                return super(Popen, self).communicate(*args, **kwargs)

        subprocess.TimeoutExpired = TimeoutExpired
        subprocess.Popen = Popen
        _EXTRA_POPEN_ARGS = {}

        warnings.warn('Timeouts are not available')
elif six.PY3:
    # In python3.2+ this all just works, no need for the madness above.
    import subprocess
    _EXTRA_POPEN_ARGS = {}

    if sys.platform == 'win32':
        # There is no implementation in piglit to make timeouts work in
        # windows, this uses the same Popen snippet as python 2 without
        # subprocess32 to mask it. Patches are welcome.
        # XXX: Should this also include cygwin?
        warnings.warn('Timeouts are not implemented on Windows.')

        class Popen(subprocess.Popen):
            """Sublcass of Popen that accepts and ignores a timeout argument."""
            def communicate(self, *args, **kwargs):
                if 'timeout' in kwargs:
                    del kwargs['timeout']
                return super(Popen, self).communicate(*args, **kwargs)

        subprocess.Popen = Popen
    elif os.name == 'posix':
        # This should work for all *nix systems, Linux, the BSDs, and OSX.
        # This speicifically creates a session group for each test, so that
        # it's children can be killed if it times out.
        _EXTRA_POPEN_ARGS = {'start_new_session': True}

# pylint: enable=wrong-import-position,wrong-import-order


__all__ = [
    'Test',
    'TestIsSkip',
    'TestRunError',
    'ValgrindMixin',
    'WindowResizeMixin',
    'is_crash_returncode',
]

# Allows timeouts to be suppressed by setting the environment variable
# PIGLIT_NO_TIMEOUT to anything that bool() will resolve as True
_SUPPRESS_TIMEOUT = bool(os.environ.get('PIGLIT_NO_TIMEOUT', False))


class TestIsSkip(exceptions.PiglitException):
    """Exception raised in is_skip() if the test is a skip."""
    def __init__(self, reason):
        super(TestIsSkip, self).__init__()
        self.reason = reason


class TestRunError(exceptions.PiglitException):
    """Exception raised if the test fails to run."""
    def __init__(self, message, status):
        super(TestRunError, self).__init__(message)
        self.status = status


def is_crash_returncode(returncode):
    """Determine whether the given process return code correspond to a
    crash.
    """
    # In python 2 NoneType and other types can be compaired, in python3 this
    # isn't allowed.
    if returncode is None:
        return False

    if sys.platform == 'win32':
        # On Windows:
        # - For uncaught exceptions the process terminates with the exception
        # code, which is usually negative
        # - MSVCRT's abort() terminates process with exit code 3
        return returncode < 0 or returncode == 3
    else:
        return returncode < 0


@six.add_metaclass(abc.ABCMeta)
class Test(object):
    """ Abstract base class for Test classes

    This class provides the framework for running tests, with several methods
    and properties that can be overwritten to produce a specialized class for
    running test suites other than piglit.

    It provides two methods for running tests, execute and run.
    execute() provides lots of features, and is invoced when running piglit
    from the command line, run() is a more basic method for running the test,
    and is called internally by execute(), but is can be useful outside of it.

    Arguments:
    command -- a value to be passed to subprocess.Popen

    Keyword Arguments:
    run_concurrent -- If True the test is thread safe. Default: False

    """
    __slots__ = ['run_concurrent', 'env', 'result', 'cwd', '_command']
    timeout = None

    def __init__(self, command, run_concurrent=False):
        assert isinstance(command, list), command

        self.run_concurrent = run_concurrent
        self._command = copy.copy(command)
        self.env = {}
        self.result = TestResult()
        self.cwd = None

    def execute(self, path, log, options):
        """ Run a test

        Run a test, but with features. This times the test, uses dmesg checking
        (if requested), and runs the logger.

        Arguments:
        path    -- the name of the test
        log     -- a log.Log instance
        options -- a dictionary containing dmesg and monitoring objects
        """
        log.start(path)
        # Run the test
        if OPTIONS.execute:
            try:
                self.result.time.start = time.time()
                options['dmesg'].update_dmesg()
                options['monitor'].update_monitoring()
                self.run()
                self.result.time.end = time.time()
                self.result = options['dmesg'].update_result(self.result)
                options['monitor'].check_monitoring()
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

    @command.setter
    def command(self, new):
        assert isinstance(new, list), 'Test.command must be a list'
        self._command = new

    @abc.abstractmethod
    def interpret_result(self):
        """Convert the raw output of the test into a form piglit understands.
        """
        if is_crash_returncode(self.result.returncode):
            self.result.result = status.CRASH
            if self.result.subtests:
                # We know because subtests are ordered that the first test with
                # a status of NOTRUN is the subtest that crashed, mark that
                # test and move on.
                for k, v in six.iteritems(self.result.subtests):
                    if v == status.NOTRUN:
                        self.result.subtests[k] = status.CRASH
                        break
        elif self.result.returncode != 0:
            if self.result.result == status.PASS:
                self.result.result = status.WARN
            else:
                self.result.result = status.FAIL

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
                six.iteritems(OPTIONS.env), six.iteritems(self.env)))

        try:
            self.is_skip()
        except TestIsSkip as e:
            self.result.result = status.SKIP
            for each in six.iterkeys(self.result.subtests):
                self.result.subtests[each] = status.SKIP
            self.result.out = e.reason
            self.result.returncode = None
            return

        try:
            self._run_command()
        except TestRunError as e:
            self.result.result = six.text_type(e.status)
            for each in six.iterkeys(self.result.subtests):
                self.result.subtests[each] = six.text_type(e.status)
            self.result.out = six.text_type(e)
            self.result.returncode = None
            return

        self.interpret_result()

    def is_skip(self):
        """ Application specific check for skip

        If this function returns a truthy value then the current test will be
        skipped. The base version will always return False

        """
        pass

    def _run_command(self, **kwargs):
        """ Run the test command and get the result

        This method sets environment options, then runs the executable. If the
        executable isn't found it sets the result to skip.

        """
        # This allows the ReducedProcessMixin to work without having to whack
        # self.command (which should be treated as immutable), but is
        # considered private.
        command = kwargs.pop('_command', self.command)

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
        # passing this as unicode is basically broken in python2 on windows, it
        # must be passed a bytes.
        if six.PY2 and sys.platform.startswith('win32'):
            f = six.binary_type
        else:
            f = six.text_type
        _base = itertools.chain(six.iteritems(os.environ),
                                six.iteritems(OPTIONS.env),
                                six.iteritems(self.env))
        fullenv = {f(k): f(v) for k, v in _base}

        try:
            proc = subprocess.Popen(command,
                                    stdout=subprocess.PIPE,
                                    stderr=subprocess.PIPE,
                                    cwd=self.cwd,
                                    env=fullenv,
                                    universal_newlines=True,
                                    **_EXTRA_POPEN_ARGS)

            self.result.pid.append(proc.pid)
            if not _SUPPRESS_TIMEOUT:
                out, err = proc.communicate(timeout=self.timeout)
            else:
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
        except subprocess.TimeoutExpired:
            # This can only be reached if subprocess32 is present or on python
            # 3.x, since # TimeoutExpired is never raised by the python 2.7
            # fallback code.

            proc.terminate()

            # XXX: This is probably broken on windows, since os.getpgid doesn't
            # exist on windows. What is the right way to handle this?
            if proc.poll() is None:
                time.sleep(3)
                os.killpg(os.getpgid(proc.pid), signal.SIGKILL)

            # Since the process isn't running it's safe to get any remaining
            # stdout/stderr values out and store them.
            self.result.out, self.result.err = proc.communicate()

            raise TestRunError(
                'Test run time exceeded timeout value ({} seconds)\n'.format(
                    self.timeout),
                'timeout')

        # The setter handles the bytes/unicode conversion
        self.result.out = out
        self.result.err = err
        self.result.returncode = returncode

    def __eq__(self, other):
        return self.command == other.command

    def __ne__(self, other):
        return not self == other


class DummyTest(Test):
    def __init__(self, name, result):
        super(DummyTest, self).__init__([name])
        self.result.result = result

    def execute(self, path, log, options):
        pass

    def interpret_result(self):
        pass


class WindowResizeMixin(object):
    """ Mixin class that deals with spurious window resizes

    On gnome (and possible other DE's) the window manager may decide to resize
    a window. This causes the test to fail even though otherwise would not.
    This Mixin overides the _run_command method to run the test 5 times, each
    time searching for the string 'Got suprious window resize' in the output,
    if it fails to find it it will break the loop and continue.

    see: https://bugzilla.gnome.org/show_bug.cgi?id=680214

    """
    def _run_command(self, *args, **kwargs):
        """Run a test up 5 times when window resize is detected.

        Rerun the command up to 5 times if the window size changes, if it
        changes 6 times mark the test as fail and return True, which will cause
        Test.run() to return early.

        """
        for _ in range(5):
            super(WindowResizeMixin, self)._run_command(*args, **kwargs)
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
        if OPTIONS.valgrind:
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

        if OPTIONS.valgrind:
            # If the underlying test failed, simply report
            # 'skip' for this valgrind test.
            if self.result.result != 'pass' and (
                    self.result.result != 'warn' and
                    self.result.returncode != 0):
                self.result.result = 'skip'
            elif self.result.returncode == 0:
                # Test passes and is valgrind clean.
                self.result.result = 'pass'
            else:
                # Test passed but has valgrind errors.
                self.result.result = 'fail'


@six.add_metaclass(abc.ABCMeta)
class ReducedProcessMixin(object):
    """This Mixin simplifies writing Test classes that run more than one test
    in a single process.

    Although one of the benefits of piglit is it's process isolation, there are
    times that process isolation is too expensive for day to day runs, and
    running more than one test in a single process is a valid trade-off for
    decreased run times. This class helps to ease writing a Test class for such
    a purpose, while not suffering all of the drawback of the approach.

    The first way that this helps is that it provides crash detection and
    recovery, allowing a single subtest to crash
    """

    def __init__(self, command, subtests=None, **kwargs):
        assert subtests is not None
        super(ReducedProcessMixin, self).__init__(command, **kwargs)
        self._expected = subtests
        self._populate_subtests()

    def is_skip(self):
        """Skip if the length of expected is 0."""
        if not self._expected:
            raise TestIsSkip('All subtests skipped')
        super(ReducedProcessMixin, self).is_skip()

    def __find_sub(self):
        """Helper for getting the next index."""
        return len([l for l in self.result.out.split('\n')
                    if self._is_subtest(l)])

    @staticmethod
    def _subtest_name(test):
        """If the name provided isn't the subtest name, this method does."""
        return test

    def _stop_status(self):
        """This method returns the status of the test that stopped the run.

        By default this will return status.CRASH, but this may not be suitable
        for some suites, which may require special considerations and need to
        require a different status in some cases, like SKIP.
        """
        return status.CRASH

    def _run_command(self, *args, **kwargs):
        """Run the command until all of the subtests have completed or crashed.

        This method will try to run all of the subtests, resuming the run if
        it's interrupted, and combining the stdout and stderr attributes
        together for parsing later. I will separate those values with
        "\n\n====RESUME====\n\n".
        """
        super(ReducedProcessMixin, self)._run_command(*args, **kwargs)

        if not self._is_cherry():
            returncode = self.result.returncode
            out = [self.result.out]
            err = [self.result.err]
            cur_sub = self.__find_sub() or 1
            last = len(self._expected)

            while cur_sub < last:
                self.result.subtests[
                    self._subtest_name(self._expected[cur_sub - 1])] = \
                        self._stop_status()

                super(ReducedProcessMixin, self)._run_command(
                    _command=self._resume(cur_sub) + list(args), **kwargs)

                out.append(self.result.out)
                err.append(self.result.err)

                # If the index is 0 the next test failed without printing a
                # name, increase by 1 so that test will be marked crash and we
                # don't get stuck in an infinite loop, otherwise return the
                # number of tests that did complete.
                cur_sub += self.__find_sub() or 1

            if not self._is_cherry():
                self.result.subtests[
                    self._subtest_name(self._expected[cur_sub - 1])] = \
                        self._stop_status()

            # Restore and keep the original returncode (so that it remains a
            # non-pass, since only one test might fail and the resumed part
            # might return 0)
            self.result.returncode = returncode
            self.result.out = '\n\n====RESUME====\n\n'.join(out)
            self.result.err = '\n\n====RESUME====\n\n'.join(err)

    def _is_cherry(self):
        """Method used to determine if rerunning is required.

        If this returns False then the rerun path will be entered, otherwise
        _run_command is effectively a bare call to super().

        Classes using this mixin may need to overwrite this if the binary
        they're calling can stop prematurely but return 0.
        """
        return self.result.returncode == 0

    def _populate_subtests(self):
        """Default implementation of subtest prepopulation.

        It may be necissary to override this depending on the subtest format.
        """
        self.result.subtests.update({x: status.NOTRUN for x in self._expected})

    @abc.abstractmethod
    def _resume(self, current):
        """Method that defines how to resume the case if it crashes.

        This method will be provided with a completed count, which is the index
        into self._expected of the first subtest that hasn't been run. This
        method should return the command to restart, and the ReduceProcessMixin
        will handle actually restarting the the process with the new command.
        """

    @abc.abstractmethod
    def _is_subtest(self, line):
        """Determines if a line in stdout contains a subtest name.

        This method is used during the resume detection phase of the
        _run_command method to determine how many subtests have successfully
        been run.

        Should simply return True if the line reprents a test starting, or
        False if it does not.
        """
