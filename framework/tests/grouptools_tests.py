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

"""Module with tests for grouptools."""

from __future__ import print_function

import nose.tools as nt

import framework.grouptools as grouptools
import framework.tests.utils as utils

doc_formatter = utils.DocFormatter({  # pylint: disable=invalid-name
    'seperator': grouptools.SEPARATOR,
})


@utils.nose_generator
def generate_tests():
    """Generate tests for the groups tools module.

    This cannot test all corners of the more complicated members.

    """
    data = [
        ('testname', grouptools.testname, grouptools.join('g1', 'g2', 't1'),
         't1'),
        ('groupname', grouptools.groupname, grouptools.join('g1', 'g2', 't1'),
         grouptools.join('g1', 'g2')),
        ('splitname', grouptools.splitname, grouptools.join('g1', 'g2', 't1'),
         (grouptools.join('g1', 'g2'), 't1')),
        ('commonprefix', grouptools.commonprefix,
         [grouptools.join('g1', 'g2', '1'), grouptools.join('g1', 'g2', '2')],
         grouptools.join('g1', 'g2', '')),
        ('split', grouptools.split, grouptools.join('g1', 'g2', 't1'),
         ['g1', 'g2', 't1']),
    ]

    test = lambda f, i, e: nt.assert_equal(f(i), e)

    for name, func, args, expect in data:
        test.description = 'grouptools.{}: works'.format(name)
        yield test, func, args, expect


def test_grouptools_join():
    """grouptools.join: works correctly."""
    # XXX: this hardcoded / needs to be fixed
    nt.assert_equal(grouptools.join('g1', 'g2'),
                    grouptools.SEPARATOR.join(['g1', 'g2']))


@doc_formatter
def test_grouptools_join_notrail():
    """grouptools.join: doesn't add trailing {seperator} with empty element"""
    test = grouptools.join('g1', 'g2', '')
    nt.ok_(not test.endswith(grouptools.SEPARATOR), msg=test)


def test_split_input_empty():
    """grouptools.split: an empty input returns []."""
    nt.assert_equal(grouptools.split(''), [])


@doc_formatter
def test_from_path_posix():
    """grouptools.from_path: converts / to {seperator} in posix paths."""
    # Since we already have tests for grouptools.join we can trust it to do the
    # right thing here. This also means that the test doesn't need to be
    # updated if the separator is changed.
    nt.assert_equal(grouptools.from_path('foo/bar'),
                    grouptools.join('foo', 'bar'))


@doc_formatter
def test_from_path_nt():
    """grouptools.from_path: converts \\ to {seperator} in nt paths."""
    nt.assert_equal(grouptools.from_path('foo\\bar'),
                    grouptools.join('foo', 'bar'))


def test_from_path_dot():
    """grouptools.from_path: should convert '.' into ''."""
    nt.assert_equal(grouptools.from_path('.'), '')
