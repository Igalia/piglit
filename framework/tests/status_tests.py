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
import framework.status as status

# Statuses from worst to last. NotRun is intentionally not in this list and
# tested separately because of upcoming features for it
STATUSES = ["notrun", "pass", "dmesg-warn", "warn", "dmesg-fail", "fail",
            "crash"]

# Create lists of fixes and regressions programmatically based on the STATUSES
# list. This means less code, and easier expansion changes.
REGRESSIONS = itertools.combinations(STATUSES, 2)
FIXES = itertools.combinations(reversed(STATUSES), 2)

# all statuses except pass are problems
PROBLEMS = STATUSES[1:]


def is_regression(x, y):
    # Test for regressions
    assert status.status_lookup(x) < status.status_lookup(y)


def is_fix(x, y):
    # Test for fix
    assert status.status_lookup(x) > status.status_lookup(y)


def is_equivalent(x, y):
    # Test if status is equivalent. Note that this does not mean 'same', two
    # statuses could be equivalent in terms of fixes and regressions, but that
    # doesn't require that they are the same status
    assert status.status_lookup(x) == status.status_lookup(y)


def is_not_equivalent(x, y):
    # Test that status is not equivalent. 
    assert status.status_lookup(x) != status.status_lookup(y)


def test_is_regression():
    # Generate all tests for regressions
    for x, y in REGRESSIONS:
        yield is_regression, x, y


def test_is_fix():
    # Generates all tests for fixes
    for x, y in FIXES:
        yield is_fix, x, y


def test_is_equivalent():
    # test the assertion that NotRun, Pass and Skip should be considered
    # equivalent for regression testing.
    for x, y in itertools.izip(STATUSES, STATUSES):
        yield is_equivalent, x, y


def test_is_change():
    for x, y in itertools.permutations(STATUSES, 2):
        yield is_not_equivalent, x, y
