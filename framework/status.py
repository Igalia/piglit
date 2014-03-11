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

NotRun
skip
pass
dmesg-warn
warn
dmesg-fail
fail
crash
timeout

(NotRun, pass, skip) are considered equivalent for regression testing.

The motivation is if you accidentally expose a feature that doesn't work,
you'll get skip->fail, which is a regression. If you disable the feature,
you'll get fail->skip, which is a fix.

NotRun->fail should also be considered a regression for you not to miss
new failing tests.

The formula for determining regressions is:
  max(old_status, pass) < new_status

The formula for determining fixes is:
  old_status > max(new_status, pass)

"""


def status_lookup(status):
    """ Provided a string return a status object instance

    When provided a string that corresponds to a key in it's status_dict
    variable, this function returns a status object instance. If the string
    does not correspond to a key it will raise an exception

    """
    status_dict = {'skip': Skip,
                   'pass': Pass,
                   'warn': Warn,
                   'fail': Fail,
                   'crash': Crash,
                   'dmesg-warn': DmesgWarn,
                   'dmesg-fail': DmesgFail,
                   'notrun': NotRun}

    try:
        return status_dict[status]()
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
    """
    A simple class for representing the output values of tests.

    This class implements a flyweight pattern, limiting the memory footprint of
    initializing thousands of these objects.

    This is a base class, and should not be directly called. Instead a child
    class should be created and called, there are many provided in this module.

    """
    # Using __slots__ allows us to implement the flyweight pattern, limiting
    # the memory consumed for creating tens of thousands of these objects.
    __slots__ = ['name', 'value', 'fraction']

    name = None
    value = None
    fraction = (0, 1)

    def __init__(self):
        raise NotImplementedError

    def __repr__(self):
        return self.name

    def __str__(self):
        return str(self.name)

    def __unicode__(self):
        return unicode(self.name)

    def __lt__(self, other):
        return int(self) < int(other)

    def __le__(self, other):
        return int(self) <= int(other)

    def __eq__(self, other):
        return int(self) == int(other)

    def __ne__(self, other):
        return int(self) != int(other)

    def __ge__(self, other):
        return int(self) >= int(other)

    def __gt__(self, other):
        return int(self) > int(other)

    def __int__(self):
        return self.value


class NotRun(Status):
    name = 'Not Run'
    value = 0
    fraction = (0, 0)

    def __init__(self):
        pass


class Skip(Status):
    name = 'skip'
    value = 5
    fraction = (0, 0)

    def __init__(self):
        pass


class Pass(Status):
    name = 'pass'
    value = 10
    fraction = (1, 1)

    def __init__(self):
        pass


class DmesgWarn(Status):
    name = 'dmesg-warn'
    value = 20

    def __init__(self):
        pass


class Warn(Status):
    name = 'warn'
    value = 25

    def __init__(self):
        pass


class DmesgFail(Status):
    name = 'dmesg-fail'
    value = 30

    def __init__(self):
        pass


class Fail(Status):
    name = 'fail'
    value = 35

    def __init__(self):
        pass


class Crash(Status):
    name = 'crash'
    value = 40

    def __init__(self):
        pass
