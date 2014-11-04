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

"""Module providing utility functions to work with piglit groups.

Instead of using posixpath (or the generic os.path) for working with tests this
module should be prefered.

Piglit groups look much like posix paths, they are '/' delimited with each
element representing a group, and the final element being the test name. Unlike
posix paths they may not start with a leading '/'.

"""

import posixpath

__all__ = [
    'join',
    'commonprefix',
    'relgroup',
    'split',
    'groupname',
    'testname',
    'splitname',
    'from_path'
]


def testname(group):
    """Return the last element of a group name.

    Provided the value 'group1/group2/test1' will provide 'test1', this
    does not enforce any rules that the final element is a test name, and can
    be used to shaved down groups.

    Analogous to os.path.basename

    """
    assert '\\' not in group, 'Groups are not paths and cannot contain \\'
    assert not group.startswith('/'), 'Groups cannot start with /'

    return posixpath.basename(group)


def groupname(group):
    """Return all groups except the last.

    Provided the value 'group1/group2/test1' will provide 'group1/group2', this
    does not enforce any rules that the final element is a test name, and can
    be used to shaved down groups.

    Analogous to os.path.dirname

    """
    assert '\\' not in group, 'Groups are not paths and cannot contain \\'
    assert not group.startswith('/'), 'Groups cannot start with /'

    return posixpath.dirname(group)


def splitname(group):
    """Split a group name, Returns tuple "(group, test)"."""
    assert '\\' not in group, 'Groups are not paths and cannot contain \\'
    assert not group.startswith('/'), 'Groups cannot start with /'

    return posixpath.split(group)


def commonprefix(args):
    """Given a list of groups, returns the longest common leading component."""
    for group in args:
        assert '\\' not in group, 'Groups are not paths and cannot contain \\'
        assert not group.startswith('/'), 'Groups cannot start with /'

    return posixpath.commonprefix(args)


def join(*args):
    """Join multiple groups together with some sanity checking.

    Prevents groups from having '/' as the leading character or from having
    '\\' in them, as these are groups not paths.

    """
    for group in args:
        assert '\\' not in group, 'Groups are not paths and cannot contain \\'
    assert not args[0].startswith('/'), 'Groups cannot start with /'

    return posixpath.join(*args)


def relgroup(large, small):
    """Find the relationship between two groups.

    This allows the comparison of two groups, and returns a string. If start
    start is longer than the group then '' is returned.

    """
    for element in {large, small}:
        assert '\\' not in element, 'Groups are not paths and cannot contain \\'
        assert not element.startswith('/'), 'Groups cannot start with /'

    if len(small) > len(large):
        return ''
    elif small == '' and large == '':
        return ''
    else:
        return posixpath.relpath(large, small)


def split(group):
    """Split the group into a list of elements.

    If input is '' return an empty list

    """
    assert '\\' not in group, 'Groups are not paths and cannot contain \\'
    assert not group.startswith('/'), 'Groups cannot start with /'
    if group == '':
        return []
    return group.split('/')


def from_path(path):
    """Create a group from a path.

    This function takes a path, and creates a group out of it.

    This safely handles both Windows and Unix style paths.

    """
    if '\\' in path:
        return path.replace('\\', '/')
    return path
