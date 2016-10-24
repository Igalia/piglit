# Copyright 2013-2016 Intel Corporation
# Copyright 2013, 2014 Advanced Micro Devices
# Copyright 2014 VMWare

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

"""Shared functions for summary generation."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import re
import operator

import six
from six.moves import zip
from itertools import repeat

# a local variable status exists, prevent accidental overloading by renaming
# the module
import framework.status as so
from framework.core import lazy_property
from framework import grouptools


class Results(object):  # pylint: disable=too-few-public-methods
    """Container object for results.

    Has the results, the names of status, and the counts of statuses.

    """
    def __init__(self, results):
        self.results = results
        self.names = Names(self)
        self.counts = Counts(self)

    def get_result(self, name):
        """Get all results for a single test.

        Replace any missing vaules with status.NOTRUN, correclty handles
        subtests.

        """
        results = []
        for res in self.results:
            try:
                results.append(res.get_result(name))
            except KeyError:
                results.append(so.NOTRUN)
        return results


class Names(object):
    """Class containing names of tests for various statuses.

    Members contain lists of sets of names that have a status.

    Each status is lazily evaluated and cached.

    """
    def __init__(self, tests):
        self.__results = tests.results

    def __diff(self, comparator, handler=None, lhs=None, rhs=None):
        """Helper for simplifying comparators using find_diffs."""
        ret = ['']
        if handler is None:
            ret.extend(find_diffs(self.__results, self.all, comparator,
                                  lhs=lhs, rhs=rhs))
        else:
            ret.extend(find_diffs(self.__results, self.all, comparator,
                                  handler=handler, lhs=lhs, rhs=rhs))
        return ret

    def __single(self, comparator):
        """Helper for simplifying comparators using find_single."""
        return find_single(self.__results, self.all, comparator)

    @lazy_property
    def all(self):
        """A set of all tests in all runs."""
        all_ = set()
        for res in self.__results:
            for key, value in six.iteritems(res.tests):
                if not value.subtests:
                    all_.add(key)
                else:
                    for subt in six.iterkeys(value.subtests):
                        all_.add(grouptools.join(key, subt))
        return all_

    @lazy_property
    def changes(self):
        def handler(names, name, prev, cur):
            """Handle missing tests.

            For changes we want literally anything where the first result
            isn't the same as the second result.

            """
            def _get(res):
                try:
                    return res.get_result(name)
                except KeyError:
                    return so.NOTRUN

            # Add any case of a != b except skip <-> notrun
            cur = _get(cur)
            prev = _get(prev)
            if cur != prev and {cur, prev} != {so.SKIP, so.NOTRUN}:
                names.add(name)

        return self.__diff(operator.ne, handler=handler)

    @lazy_property
    def problems(self):
        return self.__single(lambda x: x > so.PASS)

    @lazy_property
    def skips(self):
        # It is critical to use is not == here, otherwise so.NOTRUN will also
        # be added to this list
        return self.__single(lambda x: x is so.SKIP)

    @lazy_property
    def regressions(self):
        # By ensureing tha min(x, y) is >= so.PASS we eleminate NOTRUN and SKIP
        # from these pages
        return self.__diff(lambda x, y: x < y and min(x, y) >= so.PASS,
                           rhs=self.__results[-1])

    @lazy_property
    def fixes(self):
        # By ensureing tha min(x, y) is >= so.PASS we eleminate NOTRUN and SKIP
        # from these pages
        return self.__diff(lambda x, y: x > y and min(x, y) >= so.PASS)

    @lazy_property
    def enabled(self):
        def handler(names, name, prev, cur):
            if _result_in(name, cur) and not _result_in(name, prev):
                names.add(name)

        return self.__diff(
            lambda x, y: x is so.NOTRUN and y is not so.NOTRUN,
            handler=handler)

    @lazy_property
    def disabled(self):
        def handler(names, name, prev, cur):
            if _result_in(name, prev) and not _result_in(name, cur):
                names.add(name)

        return self.__diff(
            lambda x, y: x is not so.NOTRUN and y is so.NOTRUN,
            handler=handler)

    @lazy_property
    def incomplete(self):
        return self.__single(lambda x: x is so.INCOMPLETE)

    @lazy_property
    def all_changes(self):
        if len(self.changes) > 1:
            return set.union(*self.changes[1:])
        else:
            return set()

    @lazy_property
    def all_disabled(self):
        if len(self.disabled) > 1:
            return set.union(*self.disabled[1:])
        else:
            return set()

    @lazy_property
    def all_enabled(self):
        if len(self.enabled) > 1:
            return set.union(*self.enabled[1:])
        else:
            return set()

    @lazy_property
    def all_fixes(self):
        if len(self.fixes) > 1:
            return set.union(*self.fixes[1:])
        else:
            return set()

    @lazy_property
    def all_regressions(self):
        if len(self.regressions) > 1:
            return set.union(*self.regressions[1:])
        else:
            return set()

    @lazy_property
    def all_incomplete(self):
        if len(self.incomplete) > 1:
            return set.union(*self.incomplete)
        else:
            return self.incomplete[0]

    @lazy_property
    def all_problems(self):
        if len(self.problems) > 1:
            return set.union(*self.problems)
        else:
            return self.problems[0]

    @lazy_property
    def all_skips(self):
        if len(self.skips) > 1:
            return set.union(*self.skips)
        else:
            return self.skips[0]


class Counts(object):
    """Number of tests in each category."""
    def __init__(self, tests):
        self.__names = tests.names

    @lazy_property
    def all(self):
        return len(self.__names.all)

    @lazy_property
    def changes(self):
        return [len(x) for x in self.__names.changes]

    @lazy_property
    def problems(self):
        return [len(x) for x in self.__names.problems]

    @lazy_property
    def skips(self):
        return [len(x) for x in self.__names.skips]

    @lazy_property
    def regressions(self):
        return [len(x) for x in self.__names.regressions]

    @lazy_property
    def fixes(self):
        return [len(x) for x in self.__names.fixes]

    @lazy_property
    def enabled(self):
        return [len(x) for x in self.__names.enabled]

    @lazy_property
    def disabled(self):
        return [len(x) for x in self.__names.disabled]

    @lazy_property
    def incomplete(self):
        return [len(x) for x in self.__names.incomplete]


def escape_filename(key):
    """Avoid reserved characters in filenames."""
    return re.sub(r'[<>:"|?*#]', '_', key)


def escape_pathname(key):
    """ Remove / and \\ from names """
    return re.sub(r'[/\\]', '_', key)


def _result_in(name, result):
    """If a result (or a subtest result) exists return True, else False."""
    try:
        # This is a little hacky, but I don't know of a better way where we
        # ensure the value is truthy
        _ = result.get_result(name)
        return True
    except KeyError:
        return False


def find_diffs(results, tests, comparator, handler=lambda *a: None, lhs=None, rhs=None):
    """Generate diffs between two or more sets of results.

    Arguments:
    results -- a list of results.TestrunResult instances
    tests -- an iterable of test names. Must be iterable more than once
    comparator -- a function with the signautre f(x, y), that returns True when
                  the test should be added to the set of diffs
    lhs -- the left-hand-side result for calls to comparator.
           If not specified, results from the range results[:-1] will be used.
    rhs -- the right-hand-side result for calls to comparator.
           If not specified, results from the range results[1:] will be used.
           Note that at least one of lhs and rhs must be unspecified.

    Keyword Arguemnts:
    handler -- a function with the signature f(names, name, prev, cur). in the
               event of a KeyError while comparing the results with comparator,
               handler will be passed the (<the set of names>, <the current
               test name>, <the previous result>, <the current result>). This
               can be used to add name even when a KeyError is expected (ie,
               enabled tests).
               Default: pass

    """
    assert (lhs is None) or (rhs is None)
    diffs = [] # There can't be changes from nil -> 0
    if lhs is None:
        lhs = results[:-1]
    else:
        lhs = repeat(lhs)
    if rhs is None:
        rhs = results[1:]
    else:
        rhs = repeat(rhs)
    for prev, cur in zip(lhs, rhs):
        names = set()
        for name in tests:
            try:
                if comparator(prev.get_result(name), cur.get_result(name)):
                    names.add(name)
            except KeyError:
                handler(names, name, prev, cur)
        diffs.append(names)
    return diffs


def find_single(results, tests, func):
    """Find statuses in a single run."""
    statuses = []
    for res in results:
        names = set()
        for name in tests:
            try:
                if func(res.get_result(name)):
                    names.add(name)
            except KeyError:
                pass
        statuses.append(names)
    return statuses
