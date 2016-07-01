# encoding=utf-8
# Copyright © 2014, 2016 Intel Corporation

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

"""Tests for framework.status.

This module does not have the comprehensive tests for all of the various
combinations of comparisons between the various kinds of functions. Instead, it
just asserts that the regressions, fixes, etc lists are as expected.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import itertools

import pytest
import six

from framework import status


# Statuses from worst to last. NotRun is intentionally not in this list and
# tested separately because of upcoming features for it
STATUSES = [
    status.PASS,
    status.WARN,
    status.DMESG_WARN,
    status.FAIL,
    status.DMESG_FAIL,
    status.TIMEOUT,
    status.CRASH,
    status.INCOMPLETE,
]

# all statuses except pass are problems
PROBLEMS = STATUSES[1:]

# Create lists of fixes and regressions programmatically based on the STATUSES
# list. This means less code, and easier expansion changes.
REGRESSIONS = list(itertools.combinations(STATUSES, 2)) + \
              list(itertools.combinations([status.SKIP] + PROBLEMS, 2))
FIXES = list(itertools.combinations(reversed(STATUSES), 2)) + \
        list(itertools.combinations(list(reversed(PROBLEMS)) + [status.SKIP], 2))

# The statuses that don't cause changes when transitioning from one another
NO_OPS = [status.SKIP, status.NOTRUN]


@pytest.mark.raises(exception=status.StatusException)
def test_bad_lookup():
    """status.status_lookup: An unexepcted value raises a StatusException"""
    status.status_lookup('foobar')


@pytest.mark.raises(exception=TypeError)
def test_status_eq_raises():
    """status.Status: eq comparison to uncomparable object results in TypeError"""
    status.PASS == dict()


@pytest.mark.raises(exception=TypeError)
def test_nochangestatus_eq_raises():
    """status.NoChangeStatus: eq comparison to uncomparable type results in TypeError"""
    status.NOTRUN == dict()


@pytest.mark.raises(exception=TypeError)
def test_nochangestatus_ne_raises():
    """status.NoChangeStatus: ne comparison to uncomparable type results in TypeError"""
    status.NOTRUN != dict()


def test_status_in():
    """status.Status: A status can be looked up with 'x in y' synatx"""
    stat = status.PASS
    slist = ['pass']

    assert stat in slist


@pytest.mark.parametrize(
    'stat', itertools.chain(STATUSES, [status.SKIP, status.NOTRUN]))
def test_lookup(stat):
    status.status_lookup(stat)


@pytest.mark.parametrize('new,old', REGRESSIONS)
def test_regression(new, old):
    assert status.status_lookup(new) < status.status_lookup(old)


@pytest.mark.parametrize('new,old', FIXES)
def test_fixes(new, old):
    assert status.status_lookup(new) > status.status_lookup(old)


@pytest.mark.parametrize('new,old', itertools.permutations(STATUSES, 2))
def test_changes(new, old):
    assert status.status_lookup(new) != status.status_lookup(old)


@pytest.mark.parametrize('new,old', itertools.permutations(NO_OPS, 2))
def test_no_change(new, old):
    new = status.status_lookup(new)
    old = status.status_lookup(old)
    assert not new < old
    assert not new > old


@pytest.mark.parametrize("stat,op,expected", [
    (status.Status('Test', 0, (0, 0)), six.text_type, 'Test'),
    (status.Status('Test', 0, (0, 0)), six.binary_type, b'Test'),
    (status.Status('Test', 0, (0, 0)), int, 0),
    (status.Status('Test', 0, (0, 0)), repr, 'Status("Test", 0, (0, 0))'),
    (status.Status('Test', 0, (0, 0)), hash, hash('Test')),
    (status.NoChangeStatus('No'), hash, hash('No')),
])
def test_status_comparisons(stat, op, expected):
    """Test status.Status equality protocol."""
    assert op(stat) == expected
