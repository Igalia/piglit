# Copyright (c) 2014, 2015 Intel Corporation

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

__all__ = [
    'commonprefix',
    'from_path',
    'groupname',
    'join',
    'split',
    'splitname',
    'testname',
]

SEPARATOR = '@'


def testname(group):
    """Return the last element of a group name.

    Provided the value 'group1|group2|test1' will provide 'test1', this
    does not enforce any rules that the final element is a test name, and can
    be used to shaved down groups.

    Analogous to os.path.basename

    """
    return splitname(group)[1]


def groupname(group):
    """Return all groups except the last.

    Provided the value 'group1/group2/test1' will provide 'group1/group2', this
    does not enforce any rules that the final element is a test name, and can
    be used to shaved down groups.

    Analogous to os.path.dirname

    """
    return splitname(group)[0]


def splitname(group):
    """Split a group name, Returns tuple "(group, test)"."""
    i = group.rfind(SEPARATOR) + 1
    head, tail = group[:i], group[i:]
    head = head.rstrip(SEPARATOR)

    return head, tail


def commonprefix(args):
    """Given a list of groups, returns the longest common leading component."""
    if len(args) == 1:
        return args

    common = []

    for elems in zip(*[split(a) for a in args]):
        iter_ = iter(elems)
        first = next(iter_)
        if all(first == r for r in iter_):
            common.append(first)
        else:
            break

    return join(*common)


def join(first, *args):
    """Join multiple groups together.

    This function is implemented via string concatenation, while most
    pythonistas would use list joining, because it is accepted as better.  I
    wrote a number of implementations and timed them with timeit.  I found for
    small number of joins (2-10) that str concatenation was quite a bit faster,
    at around 100 elements list joining became faster. Since most of piglit's
    use of join is for 2-10 elements I used string concatentation, which is
    conincedently very similar to the way posixpath.join is implemented.

    """
    for group in args:
        # Only append things if the group is not empty, otherwise we'll get
        # extra SEPARATORs where we don't want them
        if group:
            if not first.endswith(SEPARATOR):
                first += SEPARATOR
            first += group

    return first


def split(group):
    """Split the group into a list of elements.

    If input is '' return an empty list

    """
    if group == '':
        return []
    return group.split(SEPARATOR)


def from_path(path):
    """Create a group from a path.

    This function takes a path, and creates a group out of it.

    This safely handles both Windows and Unix style paths.

    """
    assert isinstance(path, (str, unicode)), 'Type must be string or unicode'

    if '\\' in path:
        path = path.replace('\\', SEPARATOR)
    if '/' in path:
        path = path.replace('/', SEPARATOR)
    if '.' == path:
        return ''
    return path
