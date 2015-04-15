# Copyright (c) 2014 Intel Corperation

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

""" Tests for the Status module

Note: see framework/status.py for the authoritative list of fixes, regression,
etc

"""

from __future__ import print_function, absolute_import
import itertools

import nose.tools as nt

import framework.status as status
import framework.tests.utils as utils

# Statuses from worst to last. NotRun is intentionally not in this list and
# tested separately because of upcoming features for it
STATUSES = ["pass", "warn", "dmesg-warn", "fail", "dmesg-fail", "timeout",
            "crash"]

# all statuses except pass are problems
PROBLEMS = STATUSES[1:]

# Create lists of fixes and regressions programmatically based on the STATUSES
# list. This means less code, and easier expansion changes.
REGRESSIONS = list(itertools.combinations(STATUSES, 2)) + \
              list(itertools.combinations(["skip"] + PROBLEMS, 2))
FIXES = list(itertools.combinations(reversed(STATUSES), 2)) + \
        list(itertools.combinations(list(reversed(PROBLEMS)) + ["skip"], 2))

# The statuses that don't cause changes when transitioning from one another
NO_OPS = ('skip', 'notrun')


@utils.no_error
def initialize_status():
    """ status.Status inializes """
    status.Status('test', 1)


@utils.no_error
def initialize_nochangestatus():
    """ NoChangeStatus initializes """
    status.NoChangeStatus('test')


def compare_status_nochangestatus():
    """ Status and NoChangeStatus can be compared with < """
    status.CRASH < status.PASS


@utils.nose_generator
def test_gen_lookup():
    """ Generator that attempts to do a lookup on all statuses """
    @utils.no_error
    def test(status_):
        status.status_lookup(status_)

    for stat in STATUSES + ['skip', 'notrun']:
        test.description = "Lookup: {}".format(stat)
        yield test, stat


@nt.raises(status.StatusException)
def test_bad_lookup():
    """ A bad status raises a StatusException """
    status.status_lookup('foobar')


def test_status_in():
    """ A status can be compared to a str with `x in container` syntax """
    stat = status.PASS
    slist = ['pass']

    assert stat in slist


@utils.nose_generator
def test_is_regression():
    """ Generate all tests for regressions """
    def is_regression(new, old):
        """ Test that old -> new is a regression """
        assert status.status_lookup(new) < status.status_lookup(old)

    for new, old in REGRESSIONS:
        is_regression.description = ("Test that {0} -> {1} is a "
                                     "regression".format(old, new))
        yield is_regression, new, old


@utils.nose_generator
def test_is_fix():
    """ Generates all tests for fixes """
    def is_fix(new, old):
        """ Test that new -> old is a fix """
        assert status.status_lookup(new) > status.status_lookup(old)

    for new, old in FIXES:
        is_fix.description = ("Test that {0} -> {1} is a "
                              "fix".format(new, old))
        yield is_fix, new, old


@utils.nose_generator
def test_is_change():
    """ Test that status -> !status is a change """
    def is_not_equivalent(new, old):
        """ Test that new != old """
        assert status.status_lookup(new) != status.status_lookup(old)

    for new, old in itertools.permutations(STATUSES, 2):
        is_not_equivalent.description = ("Test that {0} -> {1} is a "
                                         "change".format(new, old))
        yield is_not_equivalent, new, old


@utils.nose_generator
def test_not_change():
    """ Skip and NotRun should not count as changes """
    def check_not_change(new, old):
        """ Check that a status doesn't count as a change 

        This checks that new < old and old < new do not return true. This is meant
        for checking skip and notrun, which we don't want to show up as regressions
        and fixes, but to go in their own special catagories.

        """
        nt.assert_false(new < old,
                        msg="{new} -> {old}, is a change "
                            "but shouldn't be".format(**locals()))
        nt.assert_false(new > old,
                        msg="{new} <- {old}, is a change "
                            "but shouldn't be".format(**locals()))

    for nochange, stat in itertools.permutations(NO_OPS, 2):
        check_not_change.description = \
            "{0} -> {1} should not be a change".format(nochange, stat)
        yield (check_not_change, status.status_lookup(nochange),
               status.status_lookup(stat))


@utils.nose_generator
def test_max_statuses():
    """ Verify that max() works between skip and non-skip statuses """
    def _max_nochange_stat(nochange, stat):
        """ max(nochange, stat) should = stat """
        nt.assert_equal(
            stat, max(nochange, stat),
            msg="max({nochange}, {stat}) = {stat}".format(**locals()))

    def _max_stat_nochange(nochange, stat):
        """ max(stat, nochange) should = stat """
        nt.assert_equal(
            stat, max(stat, nochange),
            msg="max({stat}, {nochange}) = {stat}".format(**locals()))

    for nochange, stat in itertools.product(NO_OPS, STATUSES):
        nochange = status.status_lookup(nochange)
        stat = status.status_lookup(stat)
        _max_nochange_stat.description = \
            "max({nochange}, {stat}) = {stat}".format(**locals())
        yield _max_nochange_stat, nochange, stat

        _max_stat_nochange.description = \
            "max({stat}, {nochange}) = {stat}".format(**locals())
        yield _max_stat_nochange, nochange, stat


def check_operator(obj, op, result):
    """ Test that the result of running an operator on an object is expected

    Arguments:
    obj -- an instance to test
    operator -- the operator to test on the object
    result -- the expected result

    """
    nt.assert_equal(op(obj), result)


def check_operator_equal(obj, comp, op, result):
    """ Test that the result of running an operator on an object is expected

    Arguments:
    obj -- an instance to test
    operator -- the operator to test on the object
    result -- the expected result

    """
    nt.assert_equal(op(obj, comp), result)


def check_operator_not_equal(obj, comp, op, result):
    """ Test that the result of running an operator on an object is expected

    Arguments:
    obj -- an instance to test
    operator -- the operator to test on the object
    result -- the expected result

    """
    nt.assert_not_equal(op(obj, comp), result)


@utils.nose_generator
def test_nochangestatus_magic():
    """ Test that operators unique to NoChangeStatus work """
    obj = status.NoChangeStatus('Test')
    stat = status.Status('Test', 0, (0, 0))

    # generator equality tests
    for comp, type_ in [(obj, 'status.NoChangeStatus'),
                        (stat, 'status.Status'),
                        (u'Test', 'unicode'),
                        ('Test', 'str')]:
        check_operator_equal.description = (
            'Operator eq works with type: {0} on class '
            'status.NoChangeStatus'.format(type_)
        )
        yield check_operator_equal, obj, comp, lambda x, y: x.__eq__(y), True

        check_operator_not_equal.description = (
            'Operator ne works with type: {0} on class '
            'status.NoChangeStatus'.format(type_)
        )
        yield check_operator_not_equal, obj, comp, lambda x, y: x.__ne__(y), True


@utils.nose_generator
def test_status_magic():
    """ Generator for testing magic methods in the Status class """
    obj = status.Status('foo', 0, (0, 0))
    comparitor = status.Status('bar', 10, (0, 0))

    for func, name, result in [
            (str, 'str', 'foo'),
            (unicode, 'unicode', u'foo'),
            (repr, 'repr', 'foo'),
            (int, 'int', 0)]:
        check_operator.description = 'Operator {0} works on class {1}'.format(
            str(func), 'status.Status')
        yield check_operator, obj, func, result

    for func, name in [
            (lambda x, y: x.__lt__(y), 'lt'),
            (lambda x, y: y.__gt__(x), 'gt')]:

        check_operator_equal.description = \
            'Operator {0} works on class {1} when True'.format(
                name, 'status.Status')
        yield check_operator_equal, obj, comparitor, func, True

    for func, name in [
            (lambda x, y: x.__le__(x), 'le, when ='),
            (lambda x, y: x.__le__(y), 'le, when !='),
            (lambda x, y: x.__eq__(x), 'eq'),
            (lambda x, y: x.__ge__(x), 'ge, when ='),
            (lambda x, y: y.__ge__(x), 'ge, when !='),
            (lambda x, y: x.__ne__(y), 'ne'),
            (lambda x, y: x.__eq__(x), 'eq')]:
        check_operator_not_equal.description = \
            'Operator {0} works on class {1} when False'.format(
                name, 'status.Status')
        yield check_operator_not_equal, obj, comparitor, func, False


@nt.raises(TypeError)
def test_status_eq_raises():
    """ Comparing Status and an unlike object with eq raises a TypeError """
    status.PASS == dict()


@nt.raises(TypeError)
def test_nochangestatus_eq_raises():
    """ NoChangeStatus == !(str, unicode, Status) raises TypeError """
    status.NOTRUN == dict()


@nt.raises(TypeError)
def test_nochangestatus_ne_raises():
    """ NoChangeStatus != (str, unicode, Status) raises TypeError """
    status.NOTRUN != dict()
