# Copyright (c) 2015 Intel Corporation

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

try:
    from unittest import mock
except ImportError:
    import mock

import nose.tools as nt

from . import utils
from framework import options

# pylint: disable=protected-access,line-too-long

_RETYPE = type(re.compile(''))


@utils.no_error
def test_relist_init():
    """options._ReList: inializes"""
    options._ReList()


def test_relist_init_iterable():
    """options._ReList: handles an iterable argument correctly"""
    test = options._ReList(['foo'])
    nt.assert_is_instance(test[0], _RETYPE)


def test_relist_eq():
    """options._ReList.__eq__: two ReLists are equal if their wrapped lists are equal"""
    test1 = options._ReList(['foo'])
    test2 = options._ReList(['foo'])

    nt.eq_(test1, test2)


@nt.raises(TypeError)
def test_relist_eq_other():
    """options._ReList.__eq__: raises TypeError if the other object is not an ReList"""
    test1 = options._ReList(['foo'])
    test2 = ['foo']

    nt.eq_(test1, test2)


def test_relist_ne():
    """options._ReList.__ne__: two ReLists are not equal if their wrapped lists are not equal"""
    test1 = options._ReList(['foo'])
    test2 = options._ReList(['bar'])

    nt.assert_not_equal(test1, test2)


@nt.raises(TypeError)
def test_relist_ne_other():
    """options._ReList.__ne__: raises TypeError if the other object is not an ReList"""
    test1 = options._ReList(['foo'])
    test2 = ['bar']

    nt.eq_(test1, test2)


class TestReList(object):
    """Some tests that can share state"""
    @classmethod
    def setup_class(cls):
        cls.test = options._ReList(['foo'])

    def test_getitem(self):
        """options._ReList.__getitem__: returns expected value"""
        nt.assert_is_instance(self.test[0], _RETYPE)

    def test_flags(self):
        """options._ReList.__getitem__: sets flags correctly"""
        nt.eq_(self.test[0].flags, re.IGNORECASE)

    def test_len(self):
        """options._ReList.len: returns expected values"""
        nt.eq_(len(self.test), 1)

    def test_to_json(self):
        """options._ReList.to_json: returns expected values"""
        nt.eq_(self.test.to_json(), ['foo'])


class TestReListDescriptor(object):
    @classmethod
    def setup_class(cls):
        class _Test(object):
            desc = options._ReListDescriptor('test_desc')
            notexists = options._ReListDescriptor('test_notexists')

            def __init__(self):
                self.test_desc = options._ReList()

        cls._test = _Test

    def setup(self):
        self.test = self._test()

    def test_get_exists(self):
        """options._ReListDescriptor.__get__: Returns value if it exists"""
        nt.eq_(self.test.desc, self.test.test_desc)

    def test_get_not_exists(self):
        """options._ReListDescriptor.__get__: Returns new _ReList if it doesn't exists"""
        nt.eq_(self.test.notexists, self.test.test_notexists)  # pylint: disable=no-member

    @mock.patch('framework.options.setattr', mock.Mock(side_effect=Exception))
    @nt.raises(AttributeError)
    def test_get_not_exists_fail(self):
        """options._ReListDescriptor.__get__: Raises AttributError if name doesn't exist and cant be created"""
        self.test.notexists  # pylint: disable=pointless-statement

    def test_set_relist(self):
        """options._ReListDescriptor.__set__: assigns an ReList directoy"""
        val = options._ReList(['foo'])
        self.test.desc = val
        nt.ok_(self.test.desc is val, msg='value not assigned directly')

    def test_set_other(self):
        """options._ReListDescriptor.__set__: converts other types to ReList"""
        val = options._ReList(['foo'])
        self.test.desc = ['foo']
        nt.eq_(self.test.desc, val)

    @nt.raises(NotImplementedError)
    def test_delete(self):
        """options._ReListDescriptor.__delete___: raises NotImplementedError"""
        del self.test.desc


def test_relist_insert():
    """options._ReList.len: inserts value as expected"""
    test = options._ReList(['foo'])
    obj = re.compile('bar', re.IGNORECASE)
    test.insert(0, obj)
    nt.eq_(test[0], obj)


def test_relist_delitem():
    """options._ReList.len: removes value as expected"""
    test = options._ReList(['foo'])
    del test[0]
    nt.eq_(len(test), 0)


def test_relist_setitem():
    """options._ReList.__setitem__: adds value as expected"""
    canary = re.compile('bar')

    test = options._ReList([canary])
    test[0] = 'foo'
    nt.ok_(test[0] is not canary, msg='index value 0 wasn not replaced')


def test_options_clear():
    """options.Options.clear(): resests options values to init state"""
    baseline = options._Options()

    test = options._Options()
    test.execute = False
    test.sync = True
    test.exclude_filter.append('foo')
    test.clear()

    nt.eq_(list(iter(baseline)), list(iter(test)))


def test_filterrelist_set():
    """options._FilterReList.__setitem__: replaces '/' with '.'"""
    test = options._FilterReList(['foo'])
    test[0] = 'foo/bar'
    nt.eq_(test[0].pattern, 'foo.bar')


def test_filterrelist_insert():
    """options._FilterReList.insert: replaces '/' with '.'"""
    test = options._FilterReList()
    test.insert(0, 'foo/bar')
    nt.eq_(test[0].pattern, 'foo.bar')
