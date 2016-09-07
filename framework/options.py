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

"""Stores global piglit options.

This is as close to a true global function as python gets. The only deal here
is that while you can mutate

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import os
import re

import six

__all__ = ['OPTIONS']

# pylint: disable=too-few-public-methods


_RETYPE = type(re.compile(''))


class _ReList(collections.MutableSequence):
    """A list-like container that only holds RegexObjects.

    This class behaves identically to a list, except that all objects are
    forced to be RegexObjects with a flag of re.IGNORECASE (2 if one inspects
    the object).

    If inputs do not match this object, they will be coerced to becoming such
    an object, or they assignment will fail.

    """
    def __init__(self, iterable=None):
        self._wrapped = []
        if iterable is not None:
            self.extend(iterable)

    @staticmethod
    def __compile(value):
        """Ensure that the object is properly compiled.

        If the object is not a RegexObject then compile it to one, setting the
        proper flag. If it is a RegexObject, and the flag is incorrect
        recompile it to have the proper flags. Otherwise return it.

        """
        if not isinstance(value, _RETYPE):
            return re.compile(value, re.IGNORECASE)
        elif value.flags != re.IGNORECASE:
            return re.compile(value.pattern, re.IGNORECASE)
        return value

    def __getitem__(self, index):
        return self._wrapped[index]

    def __setitem__(self, index, value):
        self._wrapped[index] = self.__compile(value)

    def __delitem__(self, index):
        del self._wrapped[index]

    def __len__(self):
        return len(self._wrapped)

    def insert(self, index, value):
        self._wrapped.insert(index, self.__compile(value))

    def __eq__(self, other):
        """Two ReList instances are the same if their wrapped list are equal."""
        if isinstance(other, _ReList):
            # There doesn't seem to be a better way to do this.
            return self._wrapped == other._wrapped  # pylint: disable=protected-access
        raise TypeError('Cannot compare _ReList and non-_ReList object')

    def __ne__(self, other):
        return not self == other

    def to_json(self):
        """Allow easy JSON serialization.

        This returns the pattern (the string or unicode used to create the re)
        of each re object in a list rather than the RegexObject itself. This is
        critical for JSON serialization, and thanks to the piglit_encoder this
        is all we need to serialize this class.

        """
        return [l.pattern for l in self]


class _FilterReList(_ReList):
    """A version of ReList that handles group madness.

    Groups are printed with '/' as a separator, but internally something else
    may be used. This version replaces '/' with '.'.

    """
    def __setitem__(self, index, value):
        # Replace '/' with '.', this solves the problem of '/' not matching
        # grouptools.SEPARATOR, but without needing to import grouptools
        super(_FilterReList, self).__setitem__(index, value.replace('/', '.'))

    def insert(self, index, value):
        super(_FilterReList, self).insert(index, value.replace('/', '.'))


class _ReListDescriptor(object):
    """A Descriptor than ensures reassignment of _{in,ex}clude_filter is an
    _ReList

    Without this some behavior's can get very strange. This descriptor is
    mostly hit by testing code, but may be of use outside of testing at some
    point.

    """
    def __init__(self, name, type_=_ReList):
        self.__name = name
        self.__type = type_

    def __get__(self, instance, cls):
        try:
            return getattr(instance, self.__name)
        except AttributeError as e:
            new = _ReList()
            try:
                setattr(instance, self.__name, new)
            except Exception:
                raise e
            return new

    def __set__(self, instance, value):
        assert isinstance(value, (collections.Sequence, collections.Set))
        if isinstance(value, self.__type):
            setattr(instance, self.__name, value)
        else:
            setattr(instance, self.__name, self.__type(value))

    def __delete__(self, instance):
        raise NotImplementedError('Cannot delete {} from {}'.format(
            self.__name, instance.__class__))


class _Options(object):  # pylint: disable=too-many-instance-attributes
    """Contains all options for a piglit run.

    This is used as a sort of global state object, kinda like piglit.conf. This
    big difference here is that this object is largely generated internally,
    and is controlled mostly through command line options rather than through
    the configuration file.

    Options are as follows:
    concurrent -- one of: ["all", "some", "none"]. Default: "some"
    execute -- False for dry run
    include_filter -- list of compiled regex which include exclusively tests
                      that match
    exclude_filter -- list of compiled regex which exclude tests that match
    valgrind -- True if valgrind is to be used
    dmesg -- True if dmesg checking is desired. This forces concurrency off
    monitored -- True if monitoring is desired. This forces concurrency off
    env -- environment variables set for each test before run
    deqp_mustpass -- True to enable the use of the deqp mustpass list feature.
    """

    include_filter = _ReListDescriptor('_include_filter', type_=_FilterReList)
    exclude_filter = _ReListDescriptor('_exclude_filter', type_=_FilterReList)

    def __init__(self):
        self.concurrent = "some"
        self.execute = True
        self._include_filter = _ReList()
        self._exclude_filter = _ReList()
        self.exclude_tests = set()
        self.valgrind = False
        self.dmesg = False
        self.monitored = False
        self.sync = False
        self.deqp_mustpass = False
        self.process_isolation = True

        # env is used to set some base environment variables that are not going
        # to change across runs, without sending them to os.environ which is
        # fickle and easy to break
        self.env = {
            'PIGLIT_SOURCE_DIR':
                os.environ.get(
                    'PIGLIT_SOURCE_DIR',
                    os.path.abspath(os.path.join(os.path.dirname(__file__),
                                                 '..')))
        }

    def clear(self):
        """Reinitialize all values to defaults."""
        self.__init__()

    def __iter__(self):
        for key, values in six.iteritems(self.__dict__):
            if not key.startswith('_'):
                yield key, values

        # Handle the attributes that have a descriptor separately
        yield 'include_filter', self.include_filter
        yield 'exclude_filter', self.exclude_filter


OPTIONS = _Options()
