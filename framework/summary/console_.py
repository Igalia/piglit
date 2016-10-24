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

"""Generate text summaries to be printed to the console."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import textwrap

import six

from framework import grouptools, backends
from .common import Results

__all__ = [
    'console',
]

_SUMMARY_TEMPLATE = textwrap.dedent("""\
    summary:
           name: {names}
           ----  {divider}
           pass: {pass_}
           fail: {fail}
          crash: {crash}
           skip: {skip}
        timeout: {timeout}
           warn: {warn}
     incomplete: {incomplete}
     dmesg-warn: {dmesg_warn}
     dmesg-fail: {dmesg_fail}
        changes: {changes}
          fixes: {fixes}
    regressions: {regressions}
          total: {total}""")


def _print_summary(results):
    """print a summary."""

    lens = [max(min(len(x.name), 20), 6) for x in results.results]
    print_template = ' '.join(
        (lambda x: '{: >' + '{0}.{0}'.format(x) + '}')(y) for y in lens)

    def status_printer(stat):
        totals = [str(x.totals['root'][stat]) for x in results.results]
        return print_template.format(*totals)

    print(_SUMMARY_TEMPLATE.format(
        names=print_template.format(*[r.name for r in results.results]),
        divider=print_template.format(*['-'*l for l in lens]),
        pass_=status_printer('pass'),
        crash=status_printer('crash'),
        fail=status_printer('fail'),
        skip=status_printer('skip'),
        timeout=status_printer('timeout'),
        warn=status_printer('warn'),
        incomplete=status_printer('incomplete'),
        dmesg_warn=status_printer('dmesg-warn'),
        dmesg_fail=status_printer('dmesg-fail'),
        changes=print_template.format(
            *[str(s) for s in results.counts.changes]),
        fixes=print_template.format(
            *[str(s) for s in results.counts.fixes]),
        regressions=print_template.format(
            *[str(s) for s in results.counts.regressions]),
        total=print_template.format(*[
            str(sum(six.itervalues(x.totals['root'])))
            for x in results.results])))


def _print_result(results, list_):
    """Takes a list of test names to print and prints the name and result."""
    for test in sorted(list_):
        print("{test}: {statuses}".format(
            test=grouptools.format(test),
            statuses=' '.join(str(r) for r in results.get_result(test))))


def console(results, mode):
    """ Write summary information to the console """
    assert mode in ['summary', 'diff', 'incomplete', 'regressions', 'all'], mode
    results = Results([backends.load(r) for r in results])

    # Print the name of the test and the status from each test run
    if mode == 'all':
        _print_result(results, results.names.all)
        _print_summary(results)
    elif mode == 'diff':
        _print_result(results, results.names.all_changes)
        _print_summary(results)
    elif mode == 'incomplete':
        _print_result(results, results.names.all_incomplete)
    elif mode == 'regressions':
        _print_result(results, results.names.all_regressions)
    elif mode == 'summary':
        _print_summary(results)
