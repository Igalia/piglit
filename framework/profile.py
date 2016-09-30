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
import importlib
import itertools
import multiprocessing
import multiprocessing.dummy
import os

import six

from framework import grouptools, exceptions, options
from framework.dmesg import get_dmesg
from framework.log import LogManager
from framework.monitoring import Monitoring
from framework.test.base import Test

__all__ = [
    'TestProfile',
    'load_test_profile',
    'merge_test_profiles'
]


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

    def filter(self, callable):
        """Filter tests out of the testdict before running.

        This method destructively filters results out of the test test
        dictionary list using the callable provided.

        Arguments:
        callable -- a callable object that returns truthy if the item remains,
                    falsy if it should be removed

        """
        for k, v in list(six.iteritems(self)):
            if not callable((k, v)):
                del self[k]

    def reorder(self, order):
        """Reorder the TestDict to match the order of the provided list."""
        new = collections.OrderedDict()
        try:
            for k in order:
                new[k] = self.__container[k]
        except KeyError:
            # If there is a name in order that isn't available in self there
            # will be a KeyError, this is expected. In this case fail
            # gracefully and report the error to the user.
            raise exceptions.PiglitFatalError(
                'Cannot reorder test: "{}", it is not in the profile.'.format(k))
        self.__container = new


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

    def _prepare_test_list(self):
        """ Prepare tests for running

        Flattens the nested group hierarchy into a flat dictionary using '/'
        delimited groups by calling self.flatten_group_hierarchy(), then
        runs it's own filters plus the filters in the self.filters name

        """
        def matches_any_regexp(x, re_list):
            return any(r.search(x) for r in re_list)

        # The extra argument is needed to match check_all's API
        def test_matches(path, test):
            """Filter for user-specified restrictions"""
            return ((not options.OPTIONS.include_filter or
                     matches_any_regexp(path, options.OPTIONS.include_filter))
                    and path not in options.OPTIONS.exclude_tests
                    and not matches_any_regexp(path, options.OPTIONS.exclude_filter))

        filters = self.filters + [test_matches]

        def check_all(item):
            """ Checks group and test name against all filters """
            path, test = item
            for f in filters:
                if not f(path, test):
                    return False
            return True

        if self.forced_test_list:
            # Remove all tests not in the test list, then reorder the tests to
            # match the testlist. This still allows additional filters to be
            # run afterwards.
            self.test_list.filter(lambda i: i[0] in self.forced_test_list)
            self.test_list.reorder(self.forced_test_list)

        # Filter out unwanted tests
        self.test_list.filter(check_all)

        if not self.test_list:
            raise exceptions.PiglitFatalError(
                'There are no tests scheduled to run. Aborting run.')

    def _pre_run_hook(self):
        """ Hook executed at the start of TestProfile.run

        To make use of this hook one will need to subclass TestProfile, and
        set this to do something, as be default it will no-op
        """
        pass

    def _post_run_hook(self):
        """ Hook executed at the end of TestProfile.run

        To make use of this hook one will need to subclass TestProfile, and
        set this to do something, as be default it will no-op
        """
        pass

    def run(self, logger, backend):
        """ Runs all tests using Thread pool

        When called this method will flatten out self.tests into
        self.test_list, then will prepare a logger, and begin executing tests
        through it's Thread pools.

        Based on the value of options.OPTIONS.concurrent it will either run all
        the tests concurrently, all serially, or first the thread safe tests
        then the serial tests.

        Finally it will print a final summary of the tests

        Arguments:
        backend -- a results.Backend derived instance

        """

        self._pre_run_hook()

        chunksize = 1

        self._prepare_test_list()
        log = LogManager(logger, len(self.test_list))

        def test(pair, this_pool=None):
            """Function to call test.execute from map"""
            name, test = pair
            with backend.write_test(name) as w:
                test.execute(name, log.get(), self.dmesg, self.monitoring)
                w(test.result)
            if self._monitoring.abort_needed:
                this_pool.terminate()

        def run_threads(pool, testlist):
            """ Open a pool, close it, and join it """
            pool.imap(lambda pair: test(pair, pool), testlist, chunksize)
            pool.close()
            pool.join()

        # Multiprocessing.dummy is a wrapper around Threading that provides a
        # multiprocessing compatible API
        #
        # The default value of pool is the number of virtual processor cores
        single = multiprocessing.dummy.Pool(1)
        multi = multiprocessing.dummy.Pool()

        try:
            if options.OPTIONS.concurrent == "all":
                run_threads(multi, six.iteritems(self.test_list))
            elif options.OPTIONS.concurrent == "none":
                run_threads(single, six.iteritems(self.test_list))
            else:
                # Filter and return only thread safe tests to the threaded pool
                run_threads(multi, (x for x in six.iteritems(self.test_list)
                                    if x[1].run_concurrent))
                # Filter and return the non thread safe tests to the single
                # pool
                run_threads(single, (x for x in six.iteritems(self.test_list)
                                     if not x[1].run_concurrent))

            log.get().summary()
        except (KeyboardInterrupt, Exception):
            # In the event that C-c is pressed, or any sort of exception would
            # generate a stacktrace, print the status line so that it's clear,
            # then die. Pressing C-c again will kill immediately.
            log.get().summary()
            raise

        self._post_run_hook()

        if self._monitoring.abort_needed:
            raise exceptions.PiglitAbort(self._monitoring.error_message)

    def filter_tests(self, function):
        """Filter out tests that return false from the supplied function

        Arguments:
        function -- a callable that takes two parameters: path, test and
                    returns whether the test should be included in the test
                    run or not.
        """
        self.filters.append(function)

    def update(self, *profiles):
        """ Updates the contents of this TestProfile instance with another

        This method overwrites key:value pairs in self with those in the
        provided profiles argument. This allows multiple TestProfiles to be
        called in the same run; which could be used to run piglit and external
        suites at the same time.

        Arguments:
        profiles -- one or more TestProfile-like objects to be merged.

        """
        for profile in profiles:
            self.test_list.update(profile.test_list)

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


def merge_test_profiles(profiles):
    """ Helper for loading and merging TestProfile instances

    Takes paths to test profiles as arguments and returns a single merged
    TestProfile instance.

    Arguments:
    profiles -- a list of one or more paths to profile files.

    """
    profile = load_test_profile(profiles.pop())
    with profile.allow_reassignment:
        for p in profiles:
            profile.update(load_test_profile(p))
    return profile
