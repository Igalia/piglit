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

""" Module provides tests for log.py module """

import sys
import itertools
from types import *  # This is a special * safe module
import nose.tools as nt
from framework.log import Log

valid_statuses = ('pass', 'fail', 'crash', 'warn', 'dmesg-warn',
                  'dmesg-fail', 'skip', 'dry-run')

def test_initialize_log():
    """ Test that Log initializes with """
    log = Log(100)
    assert log


def test_get_current_return():
    """ Test that pre_log returns a number """
    log = Log(100)

    ret = log.get_current()
    nt.assert_true(isinstance(ret, (IntType, FloatType, LongType)),
                   msg="Log.get_current() didn't return a numeric type!")


def test_mark_complete_increment_complete():
    """ Tests that Log.mark_complete() increments self.__complete """
    log = Log(100)
    ret = log.get_current()
    log.mark_complete(ret, 'pass')
    nt.assert_equal(log._Log__complete, 1,
                    msg="Log.mark_complete() did not properly incremented "
                        "Log.__current")


def check_mark_complete_increment_summary(stat):
    """ Test that passing a result to mark_complete works correctly """
    log = Log(100)
    ret = log.get_current()
    log.mark_complete(ret, stat)
    print log._Log__summary
    nt.assert_equal(log._Log__summary[stat], 1,
                    msg="Log.__summary[{}] was not properly "
                        "incremented".format(stat))


def test_mark_complete_increment_summary():
    """ Generator that creates tests for self.__summary """
    yieldable = check_mark_complete_increment_summary

    for stat in valid_statuses:
        yieldable.description = ("Test that Log.mark_complete increments "
                                 "self._summary[{}]".format(stat))
        yield yieldable, stat


def test_mark_complete_removes_complete():
    """ Test that Log.mark_complete() removes finished tests from __running """
    log = Log(100)
    ret = log.get_current()
    log.mark_complete(ret, 'pass')
    nt.assert_not_in(ret, log._Log__running,
                     msg="Running tests not removed from running list")


@nt.raises(AssertionError)
def test_mark_complete_increment_summary_bad():
    """ Only statuses in self.__summary_keys are valid for mark_complete """
    log = Log(100)
    ret = log.get_current()
    log.mark_complete(ret, 'fails')
