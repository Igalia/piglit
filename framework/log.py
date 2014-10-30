# Copyright (c) 2013 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

""" Module for terminal logging

This module provides a class LogManager, which acts as a state manager
returning BaseLog derived instances to individual tests.

"""

import sys
import abc
import itertools
import threading
import collections

__all__ = ['LogManager']


class BaseLog(object):
    """ Abstract base class for Log objects

    It provides a lock, which should be used to lock whever the shared state is
    modified, or when printing to the screen (or both).

    Arguments:
    state -- the state dict from LogManager

    """
    __metaclass__ = abc.ABCMeta

    SUMMARY_KEYS = set([
        'pass', 'fail', 'warn', 'crash', 'skip', 'dmesg-warn', 'dmesg-fail',
        'dry-run', 'timeout'])
    _LOCK = threading.Lock()

    def __init__(self, state):
        self._state = state
        self._pad = len(str(state['total']))

    @abc.abstractmethod
    def start(self, name):
        """ Called before test run starts

        This method is used to print things before the test starts

        """

    @abc.abstractmethod
    def log(self, status):
        """ Print the result of the test

        This method is run after the test completes, and is used to log the
        actual result of the test

        """

    @abc.abstractmethod
    def summary(self):
        """ Print a final summary

        This method is run at the end of the test run, and is used to print a
        final summary of the run

        """


class QuietLog(BaseLog):
    """ A logger providing minimal status output

    This logger provides only a ninja like [x/y] pass: z, fail: w ... output.

    It uses \r to print over the same line when printing to a tty. If
    sys.stdout is not a tty then it prints \n at the end of the line instead

    Arguments:
    status -- the status to print

    """
    _test_counter = itertools.count()

    def __init__(self, *args, **kwargs):
        super(QuietLog, self).__init__(*args, **kwargs)

        # If the output is a tty we can use '\r' (return, no linebreak) to
        # print over the existing line, if stdout isn't a tty that will not
        # work (and can break systems like jenkins), so we print a \n instead
        if sys.stdout.isatty():
            self._endcode = '\r'
        else:
            self._endcode = '\n'

        self.__counter = self._test_counter.next()
        self._state['running'].append(self.__counter)

    def start(self, name):
        pass

    def _log(self, status):
        """ non-locked helper for logging

        To allow for code sharing with a subclass using a non-recursive lock we
        need to share this code in a an unlocked form.

        """
        # increment complete
        self._state['complete'] += 1

        # Add to the summary dict
        assert status in self.SUMMARY_KEYS
        self._state['summary'][status] += 1

        self._print_summary()
        self._state['running'].remove(self.__counter)

    def log(self, status):
        with self._LOCK:
            self._log(status)

    def summary(self):
        with self._LOCK:
            self._print_summary()
            self._print('\n')

    def _print_summary(self):
        """ Print the summary result

        this prints '[done/total] {status}', it is a private method hidden from
        the children of this class (VerboseLog)

        """
        assert self._LOCK.locked()

        out = '[{done}/{total}] {status} Running Test(s): {running}'.format(
            done=str(self._state['complete']).zfill(self._pad),
            total=str(self._state['total']).zfill(self._pad),
            status=', '.join('{0}: {1}'.format(k, v) for k, v in
                             sorted(self._state['summary'].iteritems())),
            running=" ".join(str(x) for x in self._state['running'])
        )

        self._print(out)

    def _print(self, out):
        """ Shared print method that ensures any bad lines are overwritten """
        assert self._LOCK.locked()

        # If the new line is shorter than the old one add trailing
        # whitespace
        pad = self._state['lastlength'] - len(out)

        sys.stdout.write(out)
        if pad > 0:
            sys.stdout.write(' ' * pad)
        sys.stdout.write(self._endcode)
        sys.stdout.flush()

        # Set the length of the line just printed (minus the spaces since
        # we don't care if we leave whitespace) so that the next line will
        # print extra whitespace if necissary
        self._state['lastlength'] = len(out)


class VerboseLog(QuietLog):
    """ A more verbose version of the QuietLog

    This logger works like the quiet logger, except it doesn't overwrite the
    previous line, ever. It also prints an additional line at the start of each
    test, which the quite logger doesn't do.

    """
    def __init__(self, *args, **kwargs):
        super(VerboseLog, self).__init__(*args, **kwargs)
        self.__name = None    # the name needs to be printed by start and log

    def _print(self, out, newline=False):
        """ Print to the console, add a newline if necissary

        For the verbose logger there are times that one wants both an
        overwritten line, and a line that is static. This method adds the
        ability to print a newline charcater at the end of the line.

        This is useful for the non-status lines (running: <name>, and <status>:
        <name>), since these lines should be be overwritten, but the running
        status line should.

        """
        super(VerboseLog, self)._print(out)
        if newline:
            sys.stdout.write('\n')
            sys.stdout.flush()

    def start(self, name):
        """ Print the test that is being run next

        This printings a running: <testname> message before the test run
        starts.

        """
        with self._LOCK:
            self._print('running: {}'.format(name), newline=True)
            self.__name = name
            self._print_summary()

    def _log(self, value):
        """ Print a message after the test finishes

        This method prints <status>: <name>. It also does a little bit of magic
        before calling super() to print the status line.

        """
        self._print('{0}: {1}'.format(value, self.__name), newline=True)

        # Set lastlength to 0, this prevents printing needless padding in
        # super()
        self._state['lastlength'] = 0
        super(VerboseLog, self)._log(value)


class DummyLog(BaseLog):
    """ A Logger that does nothing """
    def __init__(self, state):
        pass

    def start(self, name):
        pass

    def log(self, status):
        pass

    def summary(self):
        pass


class LogManager(object):
    """ Creates new log objects

    Each of the log objects it creates have two accessible methods: start() and
    log(); neither method should return anything, and they must be thread safe.
    log() should accept a single argument, which is the status the test
    returned. start() should also take a single argument, the name of the test

    When the LogManager is initialized it is initialized with an argument which
    determines which Log class it will return (they are set via the LOG_MAP
    dictionary, which maps string names to class callables), it will return
    these as long as it exists.

    Arguments:
    logger -- a string name of a logger to use
    total -- the total number of test to run

    """
    LOG_MAP = {
        'quiet': QuietLog,
        'verbose': VerboseLog,
        'dummy': DummyLog,
    }

    def __init__(self, logger, total):
        assert logger in self.LOG_MAP
        self._log = self.LOG_MAP[logger]
        self._state = {
            'total': total,
            'summary': collections.defaultdict(lambda: 0),
            'lastlength': 0,
            'complete': 0,
            'running': [],
        }

    def get(self):
        """ Return a new log instance """
        return self._log(self._state)
