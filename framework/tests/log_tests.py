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

from __future__ import print_function
import sys
import collections
import nose.tools as nt
import framework.log as log
import framework.tests.utils as utils

TEST_STATE = {'total': 0, 'complete': 0, 'lastlength': 0, 'running': [],
              'summary': collections.defaultdict(lambda: 0)}


@utils.nose_generator
def test_initialize():
    """ Generate tests for class initializiation """
    check_initialize = lambda c, *a: c(*a)

    for name, class_ in [('QuiteLog', log.QuietLog),
                         ('VerboseLog', log.VerboseLog)]:
        check_initialize.description = "{} initializes".format(name)
        yield check_initialize, class_, TEST_STATE

    check_initialize.description = "LogManager initializes"
    yield check_initialize, log.LogManager, 'quiet', 100


def test_log_factory_returns_log():
    """ LogManager.get() returns a BaseLog derived instance """
    logger = log.LogManager('quiet', 100)
    log_inst = logger.get()
    assert isinstance(log_inst, log.BaseLog)


@utils.nose_generator
def test_quietlog_log_state_update():
    """ QuiteLog.log() updates shared state managed by LogManager """
    logger = log.LogManager('quiet', 100)
    log_inst = logger.get()
    log_inst.log('pass')

    # This lambda is necissary, without it description cannot be set
    check = lambda x, y: nt.assert_equal(x, y)
    for name, value in [('total', 100), ('summary', {'pass': 1}),
                        ('complete', 1)]:
        check.description = \
            'State value {0} is incremented by QuiteLog.log()'.format(
                name, value)
        yield check, logger._state[name], value


def check_for_output(func, args, file_=sys.stdout):
    """ Test that a test produced output to either stdout or stderr

    Arguments:
    func -- a callable that will produce output
    args -- any arguments to be passed to func as a container with a splat

    KeywordArguments:
    file -- should be either sys.stdout or sys.stderr. default: sys.stdout

    """
    # Ensure that the file is empty before running the test
    file_.seek(0)
    file_.truncate()

    func(*args)
    # In nose sys.stdout and sys.stderr is a StringIO object, it returns a
    # string of everything after the tell.
    assert file_.read() == ''


@utils.nose_generator
def test_print_when_expected():
    """ Generator that creates tests that ensure that methods print

    For most logger classes <class>.log() and <class>.summary() should print
    something, for some classes <class>.start() should print.

    This doesn't try to check what they are printing, just that they are
    printing, since 1) the output is very implementation dependent, and 2) it
    is subject to change.

    """
    # a list of tuples which element 1 is the name of the class and method
    # element 2 is a callable, and element 3 is a list of arguments to be
    # passed to that callable. it needs to work with the splat operator
    printing = []

    # Test QuietLog
    quiet = log.QuietLog(TEST_STATE)
    printing.append(('QuietLog.log', quiet.log, ['pass']))
    printing.append(('QuietLog.summary', quiet.summary, []))

    # Test VerboseLog
    verbose = log.VerboseLog(TEST_STATE)
    printing.append(('VerboseLog.start', verbose.start, ['a test']))
    printing.append(('VerboseLog.log', verbose.log, ['pass']))
    printing.append(('VerboseLog.summary', verbose.summary, []))

    for name, func, args in printing:
        check_for_output.description = "{} produces output".format(name)
        yield check_for_output, func, args


def check_no_output(func, args, file_=sys.stdout):
    """ file should not be written into

    This is the coralary of check_for_ouput, it takes the same arguments and
    behaves the same way. See its docstring for more info

    """
    # Ensure that the file is empty before running the test
    file_.seek(0)
    file_.truncate()

    func(*args)
    file_.seek(-1)
    assert file_.tell() == 0, 'file.tell() is at {}'.format(file_.tell())


@utils.nose_generator
def test_noprint_when_expected():
    """ Generate tests for methods that shouldn't print

    some methods shouldn't print anything, ensure that they don't

    """
    # a list of tuples which element 1 is the name of the class and method
    # element 2 is a callable, and element 3 is a list of arguments to be
    # passed to that callable. it needs to work with the splat operator
    printing = []

    # Test QuietLog
    quiet = log.QuietLog(TEST_STATE)
    printing.append(('QuietLog.start', quiet.start, ['name']))

    # Test DummyLog
    dummy = log.DummyLog(TEST_STATE)
    printing.append(('DummyLog.start', dummy.start, ['name']))
    printing.append(('DummyLog.log', dummy.log, ['pass']))
    printing.append(('DummyLog.summary', dummy.summary, []))

    for name, func, args in printing:
        check_no_output.description = "{} produces no output".format(name)
        yield check_no_output, func, args
