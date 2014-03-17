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

import itertools
import nose.tools as nt
import framework.status as status

# Statuses from worst to last. NotRun is intentionally not in this list and
# tested separately because of upcoming features for it
STATUSES = ["pass", "dmesg-warn", "warn", "dmesg-fail", "fail", "crash"]

# Create lists of fixes and regressions programmatically based on the STATUSES
# list. This means less code, and easier expansion changes.
REGRESSIONS = itertools.combinations(STATUSES, 2)
FIXES = itertools.combinations(reversed(STATUSES), 2)

# all statuses except pass are problems
PROBLEMS = STATUSES[1:]


def initialize_status():
    """ status.Status inializes """
    test = status.Status('test', 1)
    assert test


def check_lookup(stat):
    """ Lookup a status """
    stt = status.status_lookup(stat)
    assert stt


def test_gen_lookup():
    """ Generator that attempts to do a lookup on all statuses """
    yieldable = check_lookup

    for stat in STATUSES + ['skip', 'notrun']:
        yieldable.description = "Lookup: {}".format(stat)
        yield yieldable, stat


def test_status_in():
    """ A status can be compared to a str with `x in container` syntax """
    stat = status.PASS
    slist = ['pass']

    assert stat in slist


def is_regression(new, old):
    """ Test that old -> new is a regression """
    assert status.status_lookup(new) < status.status_lookup(old)


def is_fix(new, old):
    """ Test that new -> old is a fix """
    assert status.status_lookup(new) > status.status_lookup(old)


def is_not_equivalent(new, old):
    """ Test that new != old """
    assert status.status_lookup(new) != status.status_lookup(old)


def test_is_regression():
    """ Generate all tests for regressions """
    yieldable = is_regression

    for new, old in REGRESSIONS:
        yieldable.description = ("Test that {0} -> {1} is a "
                                 "regression".format(old, new))
        yield yieldable, new, old


def test_is_fix():
    """ Generates all tests for fixes """
    yieldable = is_fix

    for new, old in FIXES:
        yieldable.description = ("Test that {0} -> {1} is a "
                                 "fix".format(new, old))
        yield yieldable, new, old


def test_is_change():
    """ Test that status -> !status is a change """
    yieldable = is_not_equivalent

    for new, old in itertools.permutations(STATUSES, 2):
        yieldable.description = ("Test that {0} -> {1} is a "
                                 "change".format(new, old))
        yield yieldable, new, old


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


def test_not_change():
    """ Skip and NotRun should not count as changes """
    yieldable = check_not_change

    for nochange, stat in itertools.product(['skip', 'notrun'], STATUSES):
        yieldable.description = "{0} -> {1} should not be a change".format(
                nochange, stat)
        yield yieldable, status.status_lookup(nochange), status.status_lookup(stat)
