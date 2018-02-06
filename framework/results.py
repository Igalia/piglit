# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

""" Module for results generation """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import copy
import datetime

import six

from framework import status, exceptions, grouptools, compat

__all__ = [
    'TestrunResult',
    'TestResult',
]


class Subtests(collections.MutableMapping):
    """A dict-like object that stores Statuses as values."""
    def __init__(self, dict_=None):
        self.__container = collections.OrderedDict()

        if dict_ is not None:
            self.update(dict_)

    def __setitem__(self, name, value):
        self.__container[name.lower()] = status.status_lookup(value)

    def __getitem__(self, name):
        return self.__container[name.lower()]

    def __delitem__(self, name):
        del self.__container[name.lower()]

    def __iter__(self):
        return iter(self.__container)

    def __len__(self):
        return len(self.__container)

    def __repr__(self):
        return repr(self.__container)

    def to_json(self):
        res = dict(self)
        res['__type__'] = 'Subtests'
        return res

    @classmethod
    def from_dict(cls, dict_):
        if '__type__' in dict_:
            del dict_['__type__']

        res = cls(dict_)

        return res


class StringDescriptor(object):  # pylint: disable=too-few-public-methods
    """A Shared data descriptor class for TestResult.

    This provides a property that can be passed a str or unicode, but always
    returns a unicode object.

    """
    def __init__(self, name, default=six.text_type()):
        assert isinstance(default, six.text_type)
        self.__name = name
        self.__default = default

    def __get__(self, instance, cls):
        return getattr(instance, self.__name, self.__default)

    def __set__(self, instance, value):
        if isinstance(value, six.binary_type):
            setattr(instance, self.__name, value.decode('utf-8', 'replace'))
        elif isinstance(value, six.text_type):
            setattr(instance, self.__name, value)
        else:
            raise TypeError('{} attribute must be a unicode or bytes instance, '
                            'but was {}.'.format(self.__name, type(value)))

    def __delete__(self, instance):
        raise NotImplementedError


class TimeAttribute(object):
    """Attribute of TestResult for time.

    This attribute provides a couple of nice helpers. It stores the start and
    end time and provides methods for getting the total and delta of the times.

    """
    __slots__ = ['start', 'end']

    def __init__(self, start=0.0, end=0.0):
        self.start = start
        self.end = end

    @property
    def total(self):
        return self.end - self.start

    @property
    def delta(self):
        return str(datetime.timedelta(seconds=self.total))

    def to_json(self):
        return {
            'start': self.start,
            'end': self.end,
            '__type__': 'TimeAttribute',
        }

    @classmethod
    def from_dict(cls, dict_):
        dict_ = copy.copy(dict_)

        if '__type__' in dict_:
            del dict_['__type__']
        return cls(**dict_)


class TestResult(object):
    """An object represting the result of a single test."""
    __slots__ = ['returncode', '_err', '_out', 'time', 'command', 'traceback',
                 'environment', 'subtests', 'dmesg', '__result', 'images',
                 'exception', 'pid']
    err = StringDescriptor('_err')
    out = StringDescriptor('_out')

    def __init__(self, result=None):
        self.returncode = None
        self.time = TimeAttribute()
        self.command = str()
        self.environment = str()
        self.subtests = Subtests()
        self.dmesg = str()
        self.images = None
        self.traceback = None
        self.exception = None
        self.pid = []
        if result:
            self.result = result
        else:
            self.__result = status.NOTRUN

    @property
    def result(self):
        """Return the result of the test.

        If there are subtests return the "worst" value of those subtests. If
        there are not return the stored value of the test. There is an
        exception to this rule, and that's if the status is crash; since this
        status is set by the framework, and can be generated even when some or
        all unit tests pass.

        """
        if self.subtests and self.__result != status.CRASH:
            return max(six.itervalues(self.subtests))
        return self.__result

    @property
    def raw_result(self):
        """Get the result of the test without taking subtests into account."""
        return self.__result

    @result.setter
    def result(self, new):
        try:
            self.__result = status.status_lookup(new)
        except exceptions.PiglitInternalError as e:
            raise exceptions.PiglitFatalError(str(e))

    def to_json(self):
        """Return the TestResult as a json serializable object."""
        obj = {
            '__type__': 'TestResult',
            'command': self.command,
            'environment': self.environment,
            'err': self.err,
            'out': self.out,
            'result': self.result,
            'returncode': self.returncode,
            'subtests': self.subtests.to_json(),
            'time': self.time.to_json(),
            'exception': self.exception,
            'traceback': self.traceback,
            'dmesg': self.dmesg,
            'pid': self.pid,
        }
        return obj

    @classmethod
    def from_dict(cls, dict_):
        """Load an already generated result in dictionary form.

        This is used as an alternate constructor which converts an existing
        dictionary into a TestResult object. It converts a key 'result' into a
        status.Status object

        """
        # pylint will say that assining to inst.out or inst.err is a non-slot
        # because self.err and self.out are descriptors, methods that act like
        # variables. Just silence pylint
        # pylint: disable=assigning-non-slot
        inst = cls()

        for each in ['returncode', 'command', 'exception', 'environment',
                     'traceback', 'dmesg', 'pid', 'result']:
            if each in dict_:
                setattr(inst, each, dict_[each])

        # Set special instances
        if 'subtests' in dict_:
            inst.subtests = Subtests.from_dict(dict_['subtests'])
        if 'time' in dict_:
            inst.time = TimeAttribute.from_dict(dict_['time'])

        # out and err must be set manually to avoid replacing the setter
        if 'out' in dict_:
            inst.out = dict_['out']
        if 'err' in dict_:
            inst.err = dict_['err']

        return inst

    def update(self, dict_):
        """Update the results and subtests fields from a piglit test.

        Native piglit tests output their data as valid json, and piglit uses
        the json module to parse this data. This method consumes that raw
        dictionary data and updates itself.

        """
        if 'result' in dict_:
            self.result = dict_['result']
        elif 'subtest' in dict_:
            self.subtests.update(dict_['subtest'])


@compat.python_2_bool_compatible
class Totals(dict):
    def __init__(self, *args, **kwargs):
        super(Totals, self).__init__(*args, **kwargs)
        for each in status.ALL:
            each = six.text_type(each)
            if each not in self:
                self[each] = 0

    def __bool__(self):
        # Since totals are prepopulated, calling 'if not <Totals instance>'
        # will always result in True, this will cause it to return True only if
        # one of the values is not zero
        for each in six.itervalues(self):
            if each != 0:
                return True
        return False

    def to_json(self):
        """Convert totals to a json object."""
        result = copy.copy(self)
        result['__type__'] = 'Totals'
        return result

    @classmethod
    def from_dict(cls, dict_):
        """Convert a dictionary into a Totals object."""
        tots = cls(dict_)
        if '__type__' in tots:
            del tots['__type__']
        return tots


class TestrunResult(object):
    """The result of a single piglit run."""
    def __init__(self):
        self.name = None
        self.uname = None
        self.options = {}
        self.glxinfo = None
        self.wglinfo = None
        self.clinfo = None
        self.lspci = None
        self.time_elapsed = TimeAttribute()
        self.tests = collections.OrderedDict()
        self.totals = collections.defaultdict(Totals)

    def get_result(self, key):
        """Get the result of a test or subtest.

        If neither a test nor a subtest instance exist, then raise the original
        KeyError generated from looking up <key> in the tests attribute. It is
        the job of the caller to handle this error.

        Arguments:
        key -- the key name of the test to return

        """
        try:
            return self.tests[key].result
        except KeyError as e:
            name, test = grouptools.splitname(key)
            try:
                return self.tests[name].subtests[test]
            except KeyError:
                raise e

    def calculate_group_totals(self):
        """Calculate the number of passes, fails, etc at each level."""
        for name, result in six.iteritems(self.tests):
            # If there are subtests treat the test as if it is a group instead
            # of a test.
            if result.subtests:
                for res in six.itervalues(result.subtests):
                    res = str(res)
                    temp = name

                    self.totals[temp][res] += 1
                    while temp:
                        temp = grouptools.groupname(temp)
                        self.totals[temp][res] += 1
                    self.totals['root'][res] += 1
            else:
                res = str(result.result)
                while name:
                    name = grouptools.groupname(name)
                    self.totals[name][res] += 1
                self.totals['root'][res] += 1

    def to_json(self):
        if not self.totals:
            self.calculate_group_totals()
        rep = copy.copy(self.__dict__)
        rep['tests'] = collections.OrderedDict((k, t.to_json())
                       for k, t in six.iteritems(self.tests))
        rep['__type__'] = 'TestrunResult'
        return rep

    @classmethod
    def from_dict(cls, dict_, _no_totals=False):
        """Convert a dictionary into a TestrunResult.

        This method is meant to be used for loading results from json or
        similar formats

        _no_totals is not meant to be used externally, it allows us to control
        the generation of totals when loading old results formats.

        """
        res = cls()
        for name in ['name', 'uname', 'options', 'glxinfo', 'wglinfo', 'lspci',
                     'results_version', 'clinfo']:
            value = dict_.get(name)
            if value:
                setattr(res, name, value)

        # Since this is used to load partial metadata when writing final test
        # results there is no guarantee that this will have a "time_elapsed"
        # key
        if 'time_elapsed' in dict_:
            setattr(res, 'time_elapsed',
                    TimeAttribute.from_dict(dict_['time_elapsed']))
        res.tests = collections.OrderedDict((n, TestResult.from_dict(t))
                    for n, t in six.iteritems(dict_['tests']))

        if not 'totals' in dict_ and not _no_totals:
            res.calculate_group_totals()
        else:
            res.totals = {n: Totals.from_dict(t) for n, t in
                          six.iteritems(dict_['totals'])}

        return res
