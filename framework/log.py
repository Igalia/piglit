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
#

import sys
import collections
from .threads import synchronized_self


class Log(object):
    """ Print Progress to stdout

    Arguments:
    total -- The total number of tests to run.

    """
    def __init__(self, total, verbose):
        self.__total = total
        self.__complete = 0
        self.__running = []
        self.__generator = (x for x in xrange(self.__total))
        self.__pad = len(str(self.__total))
        self.__summary_keys = set(['pass', 'fail', 'warn', 'crash', 'skip',
                                   'dmesg-warn', 'dmesg-fail', 'dry-run',
                                   'timeout'])
        self.__summary = collections.defaultdict(lambda: 0)
        self.__lastlength = 0
        self.__tty = sys.stdout.isatty()

        self.__output = "[{percent}] {summary} {running}"

        # Some automation tools (e.g, Jenkins) will buffer all output until
        # newline, so don't use carriage return character if the stdout is not
        # a TTY.
        if self.__tty:
            self.__output += "\r"
        else:
            self.__output += "\n"

        if verbose:
            self.__output = "{result} :: {name}\n" + self.__output

        self.__summary_output = "[{percent}] {summary}\n"

    def _write_output(self, output):
        """ write the output to stdout, ensuring any previous line is cleared """

        if self.__tty:
            length = len(output)
            if self.__lastlength > length:
                output = "{0}{1}{2}".format(output[:-1],
                                            " " * (self.__lastlength - length),
                                            output[-1])

            self.__lastlength = length

        sys.stdout.write(output)

        # Need to flush explicitly, otherwise it all gets buffered without a
        # newline.
        sys.stdout.flush()

    def _summary(self):
        """ return a summary of the statuses """
        return ", ".join("{0}: {1}".format(k, self.__summary[k])
                         for k in sorted(self.__summary))

    def _running(self):
        """ return running tests """
        return "Running Test(s): {}".format(
            " ".join([str(x).zfill(self.__pad) for x in self.__running]))

    def _percent(self):
        """ return the percentage of tess completed """
        return "{0}/{1}".format(str(self.__complete).zfill(self.__pad),
                                str(self.__total).zfill(self.__pad))

    def __print(self, name, result):
        """ Do the actual printing """
        self._write_output(self.__output.format(
            percent=self._percent(),
            running=self._running(),
            summary=self._summary(),
            name=name,
            result=result))

    @synchronized_self
    def log(self, name, result, value):
        """ Print to the screen

        Works by moving the cursor back to the front of the line and printing
        over it.

        Arguments:
        name -- the name of the test
        result -- the result of the test
        value -- the number of the test to remove

        """
        assert result in self.__summary_keys
        self.__print(name, result)

        # Mark running complete
        assert value in self.__running
        self.__running.remove(value)

        # increment the number of completed tests
        self.__complete += 1

        assert result in self.__summary_keys
        self.__summary[result] += 1

    @synchronized_self
    def pre_log(self, running=None):
        """ Hook to run before log()

        Returns a new number to know what processes are running, if running is
        set it will print a running message for the test

        Keyword Arguments:
        running -- the name of a test to print is running. If Falsy then
                   nothing will be printed. Default: None

        """
        if running:
            self.__print(running, 'running')

        x = self.__generator.next()
        self.__running.append(x)
        return x

    def summary(self):
        self._write_output(self.__summary_output.format(
            percent=self._percent(),
            summary=self._summary()))
