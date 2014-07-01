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

from types import *  # This is a special * safe module
import nose.tools as nt
from framework.log import Log
import framework.tests.utils as utils

valid_statuses = ('pass', 'fail', 'crash', 'warn', 'dmesg-warn',
                  'dmesg-fail', 'skip', 'dry-run')

def test_initialize_log_terse():
    """ Test that Log initializes with verbose=False """
    log = Log(100, False)
    assert log


def test_initialize_log_verbose():
    """ Test that Log initializes with verbose=True """
    log = Log(100, True)
    assert log


def test_pre_log_return():
    """ Test that pre_log returns a number """
    log = Log(100, False)

    ret = log.pre_log()
    nt.assert_true(isinstance(ret, (IntType, FloatType, LongType)),
                   msg="Log.pre_log() didn't return a numeric type!")


def test_log_increment_complete():
    """ Tests that Log.log() increments self.__complete """
    log = Log(100, False)
    ret = log.pre_log()
    log.log('test', 'pass', ret)
    nt.assert_equal(log._Log__complete, 1,
                    msg="Log.log() did not properly incremented Log.__current")


def check_log_increment_summary(stat):
    """ Test that passing a result to log works correctly """
    log = Log(100, False)
    ret = log.pre_log()
    log.log('test', stat, ret)
    print log._Log__summary
    nt.assert_equal(log._Log__summary[stat], 1,
                    msg="Log.__summary[{}] was not properly "
                        "incremented".format(stat))


@utils.nose_generator
def test_log_increment_summary():
    """ Generator that creates tests for self.__summary """
    for stat in valid_statuses:
        check_log_increment_summary.description = \
            "Test that Log.log increments self._summary[{}]".format(stat)
        yield check_log_increment_summary, stat


def test_log_removes_complete():
    """ Test that Log.log() removes finished tests from __running """
    log = Log(100, False)
    ret = log.pre_log()
    log.log('test', 'pass', ret)
    nt.assert_not_in(ret, log._Log__running,
                     msg="Running tests not removed from running list")


@nt.raises(AssertionError)
def test_log_increment_summary_bad():
    """ Only statuses in self.__summary_keys are valid for log """
    log = Log(100, False)
    ret = log.pre_log()
    log.log('test', 'fails', ret)
