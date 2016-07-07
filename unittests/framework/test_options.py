# Copyright (c) 2015-2016 Intel Corporation

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

"""Tests for the options module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import re

import pytest

from framework import options

# pylint: disable=protected-access
# pylint: disable=invalid-name
# pylint: disable=no-self-use

_RETYPE = type(re.compile(''))


def test_ReList_iterable_argument():
    """options._ReList: handles an iterable argument correctly"""
    test = options._ReList(['foo'])
    assert isinstance(test[0], _RETYPE)


class TestReList(object):
    """Tests for the ReList class.

    These particular tests don't mutate the state of ReList, and thus can be
    run with the same instance over and over, other tests that do mutate the
    state need a per test ReList instance.

    """
    @classmethod
    def setup_class(cls):
        cls.test = options._ReList(['foo'])

    def test_eq(self):
        """Test options._ReList.__eq__."""
        test1 = ['foo']
        test2 = options._ReList(['foo'])

        with pytest.raises(TypeError):
            assert self.test == test1

        assert self.test == test2

    def test_ne(self):
        """Test hoptions._ReList.__ne__."""
        test1 = ['bar']
        test2 = options._ReList(['bar'])

        with pytest.raises(TypeError):
            assert self.test != test1

        assert self.test != test2

    def test_getitem(self):
        """options._ReList.__getitem__: returns expected value."""
        assert isinstance(self.test[0], _RETYPE)

    def test_flags(self):
        """options._ReList.__getitem__: sets flags correctly."""
        assert self.test[0].flags & re.IGNORECASE != 0

    def test_len(self):
        """options._ReList.len: returns expected values."""
        assert len(self.test) == 1

    def test_to_json(self):
        """options._ReList.to_json: returns expected values."""
        assert self.test.to_json() == ['foo']


class TestReListMutate(object):
    """Tests for ReList that mutate state."""
    test = None

    def setup(self):
        self.test = options._ReList(['foo'])

    def test_relist_insert(self):
        """options._ReList.len: inserts value as expected"""
        obj = re.compile('bar', re.IGNORECASE)

        self.test.insert(0, obj)

        assert self.test[0] == obj

    def test_relist_delitem(self):
        """options._ReList.len: removes value as expected"""
        del self.test[0]

        assert len(self.test) == 0

    def test_relist_setitem(self):
        """options._ReList.__setitem__: replaces values"""
        sentinel = re.compile('bar')
        self.test[0] = sentinel

        # The pattern must be tested because the flags on the re object might
        # require it to be recompiled, thus they might not be the same object,
        # or even be equal according to python (though they are for the
        # purposes of this test)
        assert self.test[0].pattern == sentinel.pattern


class TestReListDescriptor(object):
    """Test the ReListDescriptor class.

    Since this class is a descriptor it needs to be attached to an object at
    the class level.

    """
    test = None

    @classmethod
    def setup_class(cls):
        """Create a test object."""
        class _Test(object):
            desc = options._ReListDescriptor('test_desc')
            notexists = options._ReListDescriptor('test_notexists')

            def __init__(self):
                self.test_desc = options._ReList()

        cls._test = _Test

    def setup(self):
        self.test = self._test()

    def test_get_exists(self):
        """options._ReListDescriptor.__get__: Returns value if it exists."""
        assert self.test.desc == self.test.test_desc

    def test_get_not_exists(self):
        """options._ReListDescriptor.__get__: Returns new _ReList if it doesn't
        exists."""
        assert self.test.notexists == self.test.test_notexists  # pylint: disable=no-member

    def test_get_not_exists_fail(self, mocker):
        """options._ReListDescriptor.__get__: Raises AttributError if name
        doesn't exist and can't be created."""
        mocker.patch('framework.options.setattr',
                     mocker.Mock(side_effect=Exception),
                     create=True)

        with pytest.raises(AttributeError):
            self.test.notexists  # pylint: disable=pointless-statement

    def test_set_relist(self):
        """options._ReListDescriptor.__set__: assigns an ReList without
        copying."""
        val = options._ReList(['foo'])
        self.test.desc = val
        assert self.test.desc is val

    def test_set_other(self):
        """options._ReListDescriptor.__set__: converts other types to ReList"""
        val = options._ReList(['foo'])
        self.test.desc = ['foo']
        assert self.test.desc == val

    def test_delete(self):
        """options._ReListDescriptor.__delete___: raises NotImplementedError"""
        with pytest.raises(NotImplementedError):
            del self.test.desc


class TestFilterReList(object):
    """Tests for FilterReList.

    provides a unique instance per test, which protects against state mutation.

    """
    test = None

    def setup(self):
        self.test = options._FilterReList(['foo'])

    def test_setitem(self):
        """options._FilterReList.__setitem__: replaces '/' with '.'."""
        self.test[0] = 'foo/bar'
        assert self.test[0].pattern == 'foo.bar'

    def test_filterrelist_insert(self):
        """options._FilterReList.insert: replaces '/' with '.'."""
        self.test.insert(0, 'foo/bar')
        assert self.test[0].pattern == 'foo.bar'


def test_options_clear():
    """options.Options.clear(): resests options values to init state."""
    baseline = options._Options()

    test = options._Options()
    test.execute = False
    test.sync = True
    test.exclude_filter.append('foo')
    test.clear()

    assert list(iter(baseline)) == list(iter(test))
