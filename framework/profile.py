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

""" Provides Profiles for test groups

Each set of tests, both native Piglit profiles and external suite integration,
are represented by a TestProfile or a TestProfile derived object.

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

from framework import grouptools, exceptions
from framework.dmesg import get_dmesg
from framework.log import LogManager
from framework.monitoring import Monitoring
from framework.test.base import Test

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
        self.filters = [re.compile(f) for f in filters]
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

    This dict lowers the names of keys by default.

    This class intentionally doesn't accept keyword arguments. This is
    intentional to avoid breakages.

    """
    def __init__(self):
        # This is because it had special __setitem__ and __getitem__ protocol
        # methods, and simply passing *args and **kwargs into self.__container
        # will bypass these methods. It will also break the ordering, since a
        # regular dictionary or keyword arguments are inherintly unordered
        #
        # This counter is incremented once when the allow_reassignment context
        # manager is opened, and decremented each time it is closed. This
        # allows stacking of the context manager
        self.__allow_reassignment = 0
        self.__container = collections.OrderedDict()

    def __setitem__(self, key, value):
        """Enforce types on set operations.

        Keys should only be strings, and values should only be more Trees
        or Tests.

        This method makes one additional requirement, it lowers the key before
        adding it. This solves a couple of problems, namely that we want to be
        able to use filesystem heirarchies as groups in some cases, and those
        are assumed to be all lowercase to avoid problems on case insensitive
        filesystems.

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

    @property
    @contextlib.contextmanager
    def allow_reassignment(self):
        """Context manager that allows keys to be reassigned.

        Normally reassignment happens in error, but sometimes one actually
        wants to do reassignment, say to add extra options in a reduced
        profile. This method allows reassignment, but only within its context,
        making it an explict choice to do so.

        It is safe to nest this contextmanager.

        It is not safe to use this context manager in a threaded application

        """
        self.__allow_reassignment += 1
        yield
        self.__allow_reassignment -= 1


class TestProfile(object):
    """ Class that holds a list of tests for execution

    This class provides a container for storing tests in either a nested
    dictionary structure (deprecated), or a flat dictionary structure with '/'
    delimited groups.

    Once a TestProfile object is created tests can be added to the test_list
    name as a key/value pair, the key should be a fully qualified name for the
    test, including it's group hierarchy and should be '/' delimited, with no
    leading or trailing '/', the value should be an exectest.Test derived
    object.

    When the test list is filled calling TestProfile.run() will set the
    execution of these tests off, and will flatten the nested group hierarchy
    of self.tests and merge it with self.test_list

    """
    def __init__(self):
        self.test_list = TestDict()
        self.forced_test_list = []
        self.filters = []
        # Sets a default of a Dummy
        self._dmesg = None
        self.dmesg = False
        self.results_dir = None
        self._monitoring = None
        self.monitoring = False

    @property
    def dmesg(self):
        """ Return dmesg """
        return self._dmesg

    @dmesg.setter
    def dmesg(self, not_dummy):
        """ Set dmesg

        Arguments:
        not_dummy -- if Truthy dmesg will try to get a PosixDmesg, if Falsy it
                     will get a DummyDmesg

        """
        self._dmesg = get_dmesg(not_dummy)

    @property
    def monitoring(self):
        """ Return monitoring """
        return self._monitoring

    @monitoring.setter
    def monitoring(self, monitored):
        """ Set monitoring

        Arguments:
        monitored -- if Truthy Monitoring will enable monitoring according the
                     defined rules

        """
        self._monitoring = Monitoring(monitored)

    @contextlib.contextmanager
    def group_manager(self, test_class, group, prefix=None, **default_args):
        """A context manager to make working with flat groups simple.

        This provides a simple way to replace add_plain_test,
        add_concurrent_test, etc. Basic usage would be to use the with
        statement to yield and adder instance, and then add tests.

        This does not provide for a couple of cases.
        1) When you need to alter the test after initialization. If you need to
           set instance.env, for example, you will need to do so manually. It
           is recommended to not use this function for that case, but to
           manually assign the test and set env together, for code clearness.
        2) When you need to use a function that modifies profile.

        Arguments:
        test_class -- a Test derived class that. Instances of this class will
                      be added to the profile.
        group -- a string or unicode that will be used as the key for the test
                 in profile.

        Keyword Arguments:
        **default_args -- any additional keyword arguments will be considered
                          default arguments to all tests added by the adder.
                          They will always be overwritten by **kwargs passed to
                          the adder function

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
            args -- arguments to be passed to the test_class constructor.
                    This must be appropriate for the underlying class

            Keyword Arguments:
            name -- If this is a a truthy value that value will be used as the
                    key for the test. If name is falsy then args will be
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

            self.test_list[lgroup] = test_class(
                args,
                **dict(itertools.chain(six.iteritems(default_args),
                                       six.iteritems(kwargs))))

        yield adder

    @property
    @contextlib.contextmanager
    def allow_reassignment(self):
        """A convenience wrapper around self.test_list.allow_reassignment."""
        with self.test_list.allow_reassignment:
            yield

    def setup(self):
        """Method to do pre-run setup."""

    def teardown(self):
        """Method to od post-run teardown."""

    def copy(self):
        """Create a copy of the TestProfile.

        This method creates a copy with references to the original instance
        (using copy.copy), except for the test_list attribute, which is copied
        using copy.deepcopy. This allows profiles to be "subclassed" by other
        profiles, without modifying the original.
        """
        new = copy.copy(self)
        new.test_list = copy.deepcopy(self.test_list)
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
    and 'tests/quick.py' are all equally valid for filename.

    This will raise a FatalError if the module doesn't exist, or if the module
    doesn't have a profile attribute.

    Arguments:
    filename -- the name of a python module to get a 'profile' from

    """
    try:
        mod = importlib.import_module('tests.{0}'.format(
            os.path.splitext(os.path.basename(filename))[0]))
        return mod.profile
    except AttributeError:
        raise exceptions.PiglitFatalError(
            'There is not profile attribute in module {}.\n'
            'Did you specify the right file?'.format(filename))
    except ImportError:
        raise exceptions.PiglitFatalError(
            'There is no test profile called "{}".\n'
            'Check your spelling?'.format(filename))


def run(profiles, logger, backend, concurrency):
    """Runs all tests using Thread pool.

    When called this method will flatten out self.tests into self.test_list,
    then will prepare a logger, and begin executing tests through it's Thread
    pools.

    Based on the value of options.OPTIONS.concurrent it will either run all the
    tests concurrently, all serially, or first the thread safe tests then the
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

    def test(name, test, profile, this_pool=None):
        """Function to call test.execute from map"""
        with backend.write_test(name) as w:
            test.execute(name, log.get(), profile.dmesg, profile.monitoring)
            w(test.result)
        if profile.monitoring.abort_needed:
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
        if p.monitoring.abort_needed:
            raise exceptions.PiglitAbort(p.monitoring.error_message)
