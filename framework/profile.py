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

"""Classes dealing with groups of Tests.

In piglit tests are grouped into "profiles", which are equivalent to "suites"
in some other testing nomenclature. A profile is a way to tell the framework
that you have a group of tests you want to run, here are the names of those
tests, and the Test instance.
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import collections
import contextlib
import copy
import importlib
import itertools
import multiprocessing
import multiprocessing.dummy
import os
import re

import six

from framework import grouptools, exceptions, status
from framework.dmesg import get_dmesg
from framework.log import LogManager
from framework.monitoring import Monitoring
from framework.test.base import Test, DummyTest

__all__ = [
    'RegexFilter',
    'TestDict',
    'TestProfile',
    'load_test_profile',
    'run',
]


class RegexFilter(object):
    """An object to be passed to TestProfile.filter.

    This object takes a list (or list-like object) of strings which it converts
    to re.compiled objects (so use raw strings for escape sequences), and acts
    as a callable for filtering tests. If a test matches any of the regex then
    it will be scheduled to run. When the inverse keyword argument is True then
    a test that matches any regex will not be scheduled. Regardless of the
    value of the inverse flag if filters is empty then the test will be run.

    Arguments:
    filters -- a list of regex compiled objects.

    Keyword Arguments:
    inverse -- Inverse the sense of the match.
    """

    def __init__(self, filters, inverse=False):
        self.filters = [re.compile(f, flags=re.IGNORECASE) for f in filters]
        self.inverse = inverse

    def __call__(self, name, _):  # pylint: disable=invalid-name
        # This needs to match the signature (name, test), since it doesn't need
        # the test instance use _.

        # If self.filters is empty then return True, we don't want to remove
        # any tests from the run.
        if not self.filters:
            return True

        if not self.inverse:
            return any(r.search(name) for r in self.filters)
        else:
            return not any(r.search(name) for r in self.filters)


class TestDict(collections.MutableMapping):
    """A special kind of dict for tests.

    This mapping lowers the names of keys by default, and enforces that keys be
    strings (not bytes) and that values are Test derived objects. It is also a
    wrapper around collections.OrderedDict.

    This class doesn't accept keyword arguments, this is intentional. This is
    because the TestDict class is ordered, and keyword arguments are unordered,
    which is a design mismatch.
    """
    def __init__(self):
        # This counter is incremented once when the allow_reassignment context
        # manager is opened, and decremented each time it is closed. This
        # allows stacking of the context manager
        self.__allow_reassignment = 0
        self.__container = collections.OrderedDict()

    def __setitem__(self, key, value):
        """Enforce types on set operations.

        Keys should only be strings, and values should only be Tests.

        This method makes one additional requirement, it lowers the key before
        adding it. This solves a couple of problems, namely that we want to be
        able to use file-system hierarchies as groups in some cases, and those
        are assumed to be all lowercase to avoid problems on case insensitive
        file-systems.
        """
        # keys should be strings
        if not isinstance(key, six.text_type):
            raise exceptions.PiglitFatalError(
                "TestDict keys must be strings, but was {}".format(type(key)))

        # Values should either be more Tests
        if not isinstance(value, Test):
            raise exceptions.PiglitFatalError(
                "TestDict values must be a Test, but was a {}".format(
                    type(value)))

        # This must be lowered before the following test, or the test can pass
        # in error if the key has capitals in it.
        key = key.lower()

        # If there is already a test of that value in the tree it is an error
        if not self.__allow_reassignment and key in self.__container:
            if self.__container[key] != value:
                error = (
                    'Further, the two tests are not the same,\n'
                    'The original test has this command:   "{0}"\n'
                    'The new test has this command:        "{1}"'.format(
                        ' '.join(self.__container[key].command),
                        ' '.join(value.command))
                )
            else:
                error = "and both tests are the same."

            raise exceptions.PiglitFatalError(
                "A test has already been assigned the name: {}\n{}".format(
                    key, error))

        self.__container[key] = value

    def __getitem__(self, key):
        """Lower the value before returning."""
        return self.__container[key.lower()]

    def __delitem__(self, key):
        """Lower the value before returning."""
        del self.__container[key.lower()]

    def __len__(self):
        return len(self.__container)

    def __iter__(self):
        return iter(self.__container)

    @contextlib.contextmanager
    def group_manager(self, test_class, group, **default_args):
        """A context manager to make working with flat groups simple.

        This provides a simple way to replace add_plain_test,
        add_concurrent_test, etc. Basic usage would be to use the with
        statement to yield and adder instance, and then add tests.

        This does not provide for a couple of cases.
        1) When you need to alter the test after initialization. If you need to
           set instance.env, for example, you will need to do so manually. It
           is recommended to not use this function for that case, but to
           manually assign the test and set env together, for code clearness.
        2) When you need to use a function that modifies the TestProfile.

        Arguments:
        test_class -- a Test derived class that. Instances of this class will
                      be added to the profile.
        group      -- a string or unicode that will be used as the key for the
                      test in profile.

        Keyword Arguments:
        **         -- any additional keyword arguments will be considered
                      default arguments to all tests added by the adder. They
                      will always be overwritten by **kwargs passed to the
                      adder function

        >>> from framework.test import PiglitGLTest
        >>> p = TestProfile()
        >>> with p.group_manager(PiglitGLTest, 'a') as g:
        ...     g(['test'])
        ...     g(['power', 'test'], 'powertest')
        """
        assert isinstance(group, six.string_types), type(group)

        def adder(args, name=None, **kwargs):
            """Helper function that actually adds the tests.

            Arguments:
            args   -- arguments to be passed to the test_class constructor.
                      This must be appropriate for the underlying class

            Keyword Arguments:
            name   -- If this is a a truthy value that value will be used as
                      the key for the test. If name is falsy then args will be
                      ' '.join'd and used as name. Default: None
            kwargs -- Any additional args will be passed directly to the test
                      constructor as keyword args.
            """
            # If there is no name, then either
            # a) join the arguments list together to make the name
            # b) use the argument string as the name
            # The former is used by the Piglit{G,C}LTest classes, the latter by
            # GleanTest
            if not name:
                if isinstance(args, list):
                    name = ' '.join(args)
                else:
                    assert isinstance(args, six.string_types)
                    name = args

            assert isinstance(name, six.string_types)
            lgroup = grouptools.join(group, name)

            self[lgroup] = test_class(
                args,
                **dict(itertools.chain(six.iteritems(default_args),
                                       six.iteritems(kwargs))))

        yield adder

    @property
    @contextlib.contextmanager
    def allow_reassignment(self):
        """Context manager that allows keys to be reassigned.

        Normally reassignment happens in error, but sometimes one actually
        wants to do reassignment, say to add extra options in a reduced
        profile. This method allows reassignment, but only within its context,
        making it an explicit choice to do so.

        It is safe to nest this contextmanager.

        This is not thread safe, or even co-routine safe.
        """
        self.__allow_reassignment += 1
        yield
        self.__allow_reassignment -= 1


class TestProfile(object):
    """Class that holds a list of tests for execution.

    This class represents a single testsuite, it has a mapping (dictionary-like
    object) of tests attached (TestDict). This is a mapping of <str>:<Test>
    (python 3 str, python 2 unicode), and the key is delimited by
    grouptools.SEPARATOR.

    The group_manager method provides a context_manager to make adding test to
    the test_list easier, by doing more validation and enforcement.
    >>> t = TestProfile()
    >>> with t.group_manager(Test, 'foo@bar') as g:
    ...     g(['foo'])

    This class does not provide a way to execute itself, instead that is
    handled by the run function in this module, which is able to process and
    run multiple TestProfile objects at once.
    """
    def __init__(self):
        self.test_list = TestDict()
        self.forced_test_list = []
        self.filters = []
        self.options = {
            'dmesg': get_dmesg(False),
            'monitor': Monitoring(False),
            'ignore_missing': False,
        }

    def setup(self):
        """Method to do pre-run setup."""

    def teardown(self):
        """Method to do post-run teardown."""

    def copy(self):
        """Create a copy of the TestProfile.

        This method creates a copy with references to the original instance
        using copy.copy. This allows profiles to be "subclassed" by other
        profiles, without modifying the original.
        """
        new = copy.copy(self)
        new.test_list = copy.copy(self.test_list)
        new.forced_test_list = copy.copy(self.forced_test_list)
        new.filters = copy.copy(self.filters)
        return new

    def itertests(self):
        """Iterate over tests while filtering.

        This iterator is non-destructive.
        """
        if self.forced_test_list:
            opts = collections.OrderedDict()
            for n in self.forced_test_list:
                if self.options['ignore_missing'] and n not in self.test_list:
                    opts[n] = DummyTest(n, status.NOTRUN)
                else:
                    opts[n] = self.test_list[n]
        else:
            opts = self.test_list  # pylint: disable=redefined-variable-type

        for k, v in six.iteritems(opts):
            if all(f(k, v) for f in self.filters):
                yield k, v


def load_test_profile(filename):
    """Load a python module and return it's profile attribute.

    All of the python test files provide a profile attribute which is a
    TestProfile instance. This loads that module and returns it or raises an
    error.

    This method doesn't care about file extensions as a way to be backwards
    compatible with script wrapping piglit. 'tests/quick', 'tests/quick.tests',
    'tests/quick.py', and 'quick' are all equally valid for filename.

    This will raise a FatalError if the module doesn't exist, or if the module
    doesn't have a profile attribute.

    Raises:
    PiglitFatalError -- if the module cannot be imported for any reason, or if
                        the module lacks a "profile" attribute.

    Arguments:
    filename -- the name of a python module to get a 'profile' from
    """
    try:
        mod = importlib.import_module('tests.{0}'.format(
            os.path.splitext(os.path.basename(filename))[0]))
    except ImportError:
        raise exceptions.PiglitFatalError(
            'Failed to import "{}", there is either something wrong with the '
            'module or it doesn\'t exist. Check your spelling?'.format(
                filename))

    try:
        return mod.profile
    except AttributeError:
        raise exceptions.PiglitFatalError(
            'There is no "profile" attribute in module {}.\n'
            'Did you specify the right file?'.format(filename))


def run(profiles, logger, backend, concurrency):
    """Runs all tests using Thread pool.

    When called this method will flatten out self.tests into self.test_list,
    then will prepare a logger, and begin executing tests through it's Thread
    pools.

    Based on the value of concurrency it will either run all the tests
    concurrently, all serially, or first the thread safe tests then the
    serial tests.

    Finally it will print a final summary of the tests.

    Arguments:
    profiles -- a list of Profile instances.
    logger   -- a log.LogManager instance.
    backend  -- a results.Backend derived instance.
    """
    chunksize = 1

    # The logger needs to know how many tests are running. Because of filters
    # there's no way to do that without making a concrete list out of the
    # filters profiles.
    profiles = [(p, list(p.itertests())) for p in profiles]
    log = LogManager(logger, sum(len(l) for _, l in profiles))

    # check that after the filters are run there are actually tests to run.
    if not any(l for _, l in profiles):
        raise exceptions.PiglitUserError('no matching tests')

    def test(name, test, profile, this_pool=None):
        """Function to call test.execute from map"""
        with backend.write_test(name) as w:
            test.execute(name, log.get(), profile.options)
            w(test.result)
        if profile.options['monitor'].abort_needed:
            this_pool.terminate()

    def run_threads(pool, profile, test_list, filterby=None):
        """ Open a pool, close it, and join it """
        if filterby:
            # Although filterby could be attached to TestProfile as a filter,
            # it would have to be removed when run_threads exits, requiring
            # more code, and adding side-effects
            test_list = (x for x in test_list if filterby(x))

        pool.imap(lambda pair: test(pair[0], pair[1], profile, pool),
                  test_list, chunksize)

    def run_profile(profile, test_list):
        """Run an individual profile."""
        profile.setup()
        if concurrency == "all":
            run_threads(multi, profile, test_list)
        elif concurrency == "none":
            run_threads(single, profile, test_list)
        else:
            assert concurrency == "some"
            # Filter and return only thread safe tests to the threaded pool
            run_threads(multi, profile, test_list,
                        lambda x: x[1].run_concurrent)

            # Filter and return the non thread safe tests to the single
            # pool
            run_threads(single, profile, test_list,
                        lambda x: not x[1].run_concurrent)
        profile.teardown()

    # Multiprocessing.dummy is a wrapper around Threading that provides a
    # multiprocessing compatible API
    #
    # The default value of pool is the number of virtual processor cores
    single = multiprocessing.dummy.Pool(1)
    multi = multiprocessing.dummy.Pool()

    try:
        for p in profiles:
            run_profile(*p)

        for pool in [single, multi]:
            pool.close()
            pool.join()
    finally:
        log.get().summary()

    for p, _ in profiles:
        if p.options['monitor'].abort_needed:
            raise exceptions.PiglitAbort(p.options['monitor'].error_message)
