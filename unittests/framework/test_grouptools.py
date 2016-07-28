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

"""Module with tests for grouptools."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import pytest

from framework import grouptools

# pylint: disable=no-self-use


class TestJoin(object):
    """Tests for the join function."""

    def test_basic(self):
        """grouptools.join: works correctly."""
        assert grouptools.join('g1', 'g2') == \
            grouptools.SEPARATOR.join(['g1', 'g2'])

    def test_no_trailing_separator(self):
        """grouptools.join: doesn't add trailing separator with empty
        element.
        """
        test = grouptools.join('g1', 'g2', '')
        assert not test.endswith(grouptools.SEPARATOR)

    def test_empty(self):
        """grouptools.join: empty values are not joined"""
        assert grouptools.join('', 'spec') == 'spec'


class TestSplit(object):
    """tests for the split function."""

    def test_basic(self):
        assert grouptools.split(grouptools.join('g', 't')) == ['g', 't']

    def test_input_empty(self):
        """grouptools.split: an empty input returns []."""
        assert grouptools.split('') == []


class TestFromPath(object):
    """Tests for the from_path function."""

    def test_nt(self):
        """grouptools.from_path: converts \\ to separator in nt paths."""
        assert grouptools.from_path('foo\\bar') == grouptools.join('foo', 'bar')

    def test_posix(self):
        """grouptools.from_path: converts / to separator in posix paths."""
        # Since we already have tests for grouptools.join we can trust it to do
        # the right thing here. This also means that the test doesn't need to
        # be updated if the separator is changed.
        assert grouptools.from_path('foo/bar') == grouptools.join('foo', 'bar')

    def test_dot(self):
        """grouptools.from_path: should convert '.' into ''."""
        assert grouptools.from_path('.') == ''


class TestCommonprefix(object):
    """tests for the common prefix function."""

    def test_empty(self):
        """grouptools.commonprefix: handles an empty value"""
        actual = grouptools.commonprefix((grouptools.join('foo', 'bar'), ''))
        assert actual == ''

    def test_none(self):
        """grouptools.commonprefix: returns '' when no values are the same"""
        assert grouptools.commonprefix(['foo', 'bar']) == ''

    def test_basic(self):
        expected = grouptools.commonprefix([grouptools.join('g1', 'g2', '1'),
                                            grouptools.join('g1', 'g2', '2')])
        assert expected == grouptools.join('g1', 'g2')


class TestFormat(object):
    """tests for the format function."""

    def test_basic(self):
        """grouptools.format: replaces grouptools.SEPARATOR with '/'"""
        test_str = grouptools.SEPARATOR.join(['foo', 'bar', 'boink'])
        assert grouptools.format(test_str) == 'foo/bar/boink'


class TestTestname(object):
    """Tests for the testname function."""

    def test_basic(self):
        assert grouptools.testname(grouptools.join('g1', 'g2', 't1')) == 't1'


class TestGroupname(object):
    """Tests for the groupname function."""

    def test_basic(self):
        assert grouptools.groupname(grouptools.join('g1', 'g2', 't1')) == \
            grouptools.join('g1', 'g2')


class TestSplitname(object):
    """Tests for the splitname function."""

    def test_basic(self):
        assert grouptools.splitname(grouptools.join('g1', 'g2', 't1')) == \
            (grouptools.join('g1', 'g2'), 't1')
