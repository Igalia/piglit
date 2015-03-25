# Copyright (c) 2013 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

""" Provides classes that represent test statuses

These classes work similar to enums in other languages. They provide a limited
set of attributes that are defined per status:

A name -- a string representation of the values
An integer -- a value that allows classes to be compared using python's rich
comparisons
A tuple representing :
  [0] -- whether the test is considered a passing status
  [1] -- whether the test should be added to the total number of tests. This
         allows statuses like 'skip' to not affect the total percentage.


Status ordering from best to worst:

pass
dmesg-warn
warn
dmesg-fail
fail
timeout
crash

SKIP and NOTRUN are not factored into regressions and fixes, they are counted
seperately. They also derive from a sublcass of Status, which always returns
False

The formula for determining regressions is:
  old_status < new_status

The formula for determining fixes is:
  old_status > new_status

"""

from __future__ import print_function, absolute_import

__all__ = ['NOTRUN',
           'PASS',
           'FAIL',
           'WARN',
           'CRASH',
           'DMESG_WARN',
           'DMESG_FAIL',
           'SKIP',
           'TIMEOUT',
           'INCOMPLETE',
           'ALL']


def status_lookup(status):
    """ Provided a string return a status object instance

    When provided a string that corresponds to a key in it's status_dict
    variable, this function returns a status object instance. If the string
    does not correspond to a key it will raise an exception

    """
    status_dict = {
        'skip': SKIP,
        'pass': PASS,
        'warn': WARN,
        'fail': FAIL,
        'crash': CRASH,
        'dmesg-warn': DMESG_WARN,
        'dmesg-fail': DMESG_FAIL,
        'notrun': NOTRUN,
        'timeout': TIMEOUT,
        'incomplete': INCOMPLETE,
    }

    try:
        return status_dict[status]
    except KeyError:
        # Raise a StatusException rather than a key error
        raise StatusException


class StatusException(LookupError):
    """ Raise this exception when a string is passed to status_lookup that
    doesn't exists

    The primary reason to have a special exception is that otherwise
    status_lookup returns a KeyError, but there are many cases where it is
    desireable to except a KeyError and have an exception path. Using a custom
    Error class here allows for more fine-grained control.

    """
    pass


class Status(object):
    """ A simple class for representing the output values of tests.

    This class creatse the necessary magic values to use python's rich
    comparison methods. This allows two objects to be compared using common
    operators like <. >, ==, etc. It also alows them to be looked up in
    containers using ``for x in []`` syntax.

    This class is meant to be immutable, it (ab)uses two tools to provide this
    psuedo-immutability: the property decorator, and the __slots__ attribute to
    1: make the three attributes getters, therefor unwritable, and 2: make
    adding additional attributes impossible

    Arguments:
    name -- the name of the status
    value -- an int used to sort statuses from best to worst (0 -> inf)
    fraction -- a tuple with two ints representing
                [0]: 1 if the status is 'passing', else 0
                [1]: 1 if the status counts toward the total number of tests,
                     else 0

    """
    __slots__ = ['__name', '__value', '__fraction']

    def __init__(self, name, value, fraction=(0, 1)):
        assert isinstance(value, int), type(value)
        # The object is immutable, so calling self.foo = foo will raise a
        # TypeError. Using setattr from the parrent object works around this.
        self.__name = name
        self.__value = value
        self.__fraction = fraction

    @property
    def name(self):
        """ Return the value of name """
        return self.__name

    @property
    def value(self):
        """ Return the sorting value """
        return self.__value

    @property
    def fraction(self):
        """ Return the totals of the test as a tuple: (<pass>. <total>) """
        return self.__fraction

    def __repr__(self):
        return self.name

    def __str__(self):
        return str(self.name)

    def __unicode__(self):
        return unicode(self.name)

    def __lt__(self, other):
        return not self.__ge__(other)

    def __le__(self, other):
        return not self.__gt__(other)

    def __eq__(self, other):
        # This must be int or status, since status comparisons are done using
        # the __int__ magic method
        if isinstance(other, (int, Status)):
            return int(self) == int(other)
        elif isinstance(other, (str, unicode)):
            return unicode(self) == unicode(other)
        raise TypeError("Cannot compare type: {}".format(type(other)))

    def __ne__(self, other):
        return not self == other

    def __ge__(self, other):
        return self.fraction[1] > other.fraction[1] or (
            self.fraction[1] == other.fraction[1] and int(self) >= int(other))

    def __gt__(self, other):
        return self.fraction[1] > other.fraction[1] or int(self) > int(other)

    def __int__(self):
        return self.value


class NoChangeStatus(Status):
    """ Special sublcass of status that overides rich comparison methods

    This special class of a Status is for use with NOTRUN and SKIP, it never
    returns that it is a pass or regression

    """
    def __init__(self, name, value=0, fraction=(0, 0)):
        super(NoChangeStatus, self).__init__(name, value, fraction)

    def __eq__(self, other):
        if isinstance(other, (str, unicode, Status)):
            return unicode(self) == unicode(other)
        raise TypeError("Cannot compare type: {}".format(type(other)))

    def __ne__(self, other):
        if isinstance(other, (str, unicode, Status)):
            return unicode(self) != unicode(other)
        raise TypeError("Cannot compare type: {}".format(type(other)))


NOTRUN = NoChangeStatus('Not Run')

SKIP = NoChangeStatus('skip')

PASS = Status('pass', 0, (1, 1))

WARN = Status('warn', 10)

DMESG_WARN = Status('dmesg-warn', 20)

FAIL = Status('fail', 30)

DMESG_FAIL = Status('dmesg-fail', 40)

TIMEOUT = Status('timeout', 50)

CRASH = Status('crash', 60)

INCOMPLETE = Status('incomplete', 100)

# A tuple (ordered, immutable) of all statuses in this module
ALL = (PASS, WARN, DMESG_WARN, FAIL, DMESG_FAIL, TIMEOUT, CRASH, INCOMPLETE,
       SKIP, NOTRUN)
