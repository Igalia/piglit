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
import inspect

import nose.tools as nt

import framework.grouptools as grouptools
import framework.tests.utils as utils


def description(desc):
    """Set description on a bound method.

    This is an ugly little mother that does something awful, it sets the
    description attribute on a bound method. This allows a bound method to be
    passed from a test generator with a description.

    The string will be formated passing self.__dict__ to str.vformat, so any
    values that should be filled in should have the same names as the
    attributes of the class

    """
    def wrapper(func):
        func.description = desc
        return func
    return wrapper


class _SharedFunctionTest(object):  # pylint: disable=too-few-public-methods
    """Magic test class."""
    def __init__(self, name, function, input_, expected, explode=False):
        # pylint: disable=too-many-arguments
        self.input = input_
        self.expected = expected
        self.name = name
        self.function = function
        self.explode = explode

    def __iter__(self):
        """Iterate over the test methods of this class.

        Like the description wrapper this is ugly, it iterates over the bound
        methods of the class looking for those who's names start with test_,
        then it uses getatter to get the coresponding method, then formats that
        description string of that method with vformat, and finally yields the
        test.

        """
        # Not exactly sure why x() needs to be called, but it does.
        wrapper = lambda x: x()

        for name, test in inspect.getmembers(self, inspect.ismethod):
            if name.startswith('test_'):
                wrapper.description = test.description.format(**self.__dict__)
                yield wrapper, test


class _GroupToolsTest(_SharedFunctionTest):
    """Not so simple class for running the same tests on multiple callables.

    Provides a set of test methods that rely on instance attributes to do the
    testing, meaning setting a few attributes creates a complete test.

    Arguments:
    input --    data to pass to the function
    expected -- the result that is expected
    name --     the name of the function being tested
    function -- the function to test
    explode --  if the function takes multiple arguments they must be passed as
                container, if explode is set to True then the container will be
                exploded when passed in

    """
    @description("grouptools.{name}: works")
    def test_functionality(self):
        """Test that the functionality of the function."""
        if not self.explode:
            nt.assert_equal(self.function(self.input), self.expected)
        else:
            nt.assert_equal(self.function(*self.input), self.expected)

    @description("grouptools.{name}: doesn't accept a leading /")
    @nt.raises(AssertionError)
    def test_assertion_slash(self):
        """Test that a leading / is an error."""
        if isinstance(self.input, (str, unicode)):
            self.function('/' + self.input)
        elif not self.explode:
            self.function(['/' + i for i in self.input])
        else:
            self.function(*['/' + i for i in self.input])

    @description("grouptools.{name}: doesn't accept \\ in groups")
    @nt.raises(AssertionError)
    def test_assertion_backslash(self):
        """Test that \\ in a group is an error."""
        if isinstance(self.input, (str, unicode)):
            self.function(self.input.replace('/', '\\'))
        elif not self.explode:
            self.function(i.replace('/', '\\') for i in self.input)
        else:
            self.function(*[i.replace('/', '\\') for i in self.input])


@utils.nose_generator
def generate_tests():
    """Generate tests for the groups tools module.

    This cannot test all corners of the more complicated members.

    """
    # pylint: disable=line-too-long
    tests = [
        ('testname', grouptools.testname, 'g1/g2/t1', 't1'),
        ('groupname', grouptools.groupname, 'g1/g2/t1', 'g1/g2'),
        ('splitname', grouptools.splitname, 'g1/g2/t1', ('g1/g2', 't1')),
        ('commonprefix', grouptools.commonprefix, ['g1/g2/1', 'g1/g2/2'], 'g1/g2/'),
        ('join', grouptools.join, ['g1/g2', 't1'], 'g1/g2/t1', True),
        ('split', grouptools.split, 'g1/g2/t1', ['g1', 'g2', 't1']),
        ('relgroup', grouptools.relgroup, ['g1/g2/g3/t1', 'g1/g2'], 'g3/t1', True)
    ]
    # pylint: enable=line-too-long

    for args in tests:
        test_class = _GroupToolsTest(*args)
        # In python3 we could use 'yield from'
        for wrapper, test in test_class:
            yield wrapper, test


def test_relgroup_small_gt_large():
    """grouptools.relgroup: if small > large return ''."""
    nt.assert_equal(grouptools.relgroup('foo', 'foobar'), '')


def test_relgroup_both_empty():
    """grouptools.relgroup: if small == '' and large == '' return ''."""
    nt.assert_equal(grouptools.relgroup('', ''), '')


def test_split_input_empty():
    """grouptools.split: an empty input returns []."""
    nt.assert_equal(grouptools.split(''), [])


def test_from_path_posix():
    """grouptools.from_path: doesn't change posixpaths."""
    nt.assert_equal(grouptools.from_path('foo/bar'), 'foo/bar')


def test_from_path_nt():
    """grouptools.from_path: converts \\ to / in nt paths."""
    nt.assert_equal(grouptools.from_path('foo\\bar'), 'foo/bar')


def test_from_path_dot():
    """grouptools.from_path: should convert '.' into ''."""
    nt.assert_equal(grouptools.from_path('.'), '')
