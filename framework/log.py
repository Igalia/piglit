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
    def __init__(self, total):
        self.__total = total
        self.__complete = 0
        self.__running = []
        self.__generator = (x for x in xrange(self.__total))
        self.__pad = len(str(self.__total))
        self.__summary_keys = set(['pass', 'fail', 'warn', 'crash', 'skip',
                                   'dmesg-warn', 'dmesg-fail', 'dry-run'])
        self.__summary = collections.defaultdict(lambda: 0)

    def _summary(self):
        return ", ".join("{0}: {1}".format(k, self.__summary[k])
                         for k in sorted(self.__summary))

    def _running(self):
        return "Running Test(s): {}".format(
            " ".join([str(x).zfill(self.__pad) for x in self.__running]))

    def _percent(self):
        return "[{0}/{1}]".format(str(self.__complete).zfill(self.__pad),
                                  str(self.__total).zfill(self.__pad))

    @synchronized_self
    def mark_complete(self, value, result):
        """ Used to mark a test as complete in the log

        Arguments:
        value -- the test number to mark complete
        result -- the result of the completed test

        """
        # Mark running complete
        assert value in self.__running
        self.__running.remove(value)

        # increment the number of completed tests
        self.__complete += 1

        assert result in self.__summary_keys
        self.__summary[result] += 1

    @synchronized_self
    def log(self):
        """ Print to the screen 

        Works by moving the cursor back to the front of the line and printing
        over it.
        
        """
        sys.stdout.write("{0} {1} {2}\r".format(
            self._percent(), self._summary(), self._running()))

        # Need to flush explicitly, otherwise it all gets buffered without a
        # newline.
        sys.stdout.flush()

    @synchronized_self
    def get_current(self):
        """ Returns a new number to know what processes are running """
        x = self.__generator.next()
        self.__running.append(x)
        return x
