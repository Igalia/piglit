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

from __future__ import print_function, absolute_import
import os
import sys
import multiprocessing
import multiprocessing.dummy
import importlib
import types
import contextlib
import itertools

from framework.dmesg import get_dmesg
from framework.log import LogManager
from framework.test.base import Test
import framework.grouptools as grouptools

__all__ = [
    'TestProfile',
    'load_test_profile',
    'merge_test_profiles'
]


class TestDict(dict):  # pylint: disable=too-few-public-methods
    """A special kind of dict for tests.

    This dict lowers the names of keys by default

    """
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
        assert isinstance(key, basestring), \
            "Keys must be strings, but was {}".format(type(key))
        # None is required to make empty assignment work:
        # foo = Tree['a']
        assert isinstance(value, (Test, types.NoneType)), \
            "Values must be either a Test, but was {}".format(type(value))

        super(TestDict, self).__setitem__(key.lower(), value)

    def __getitem__(self, key):
        """Lower the value before returning."""
        return super(TestDict, self).__getitem__(key.lower())

    def __delitem__(self, key):
        """Lower the value before returning."""
        return super(TestDict, self).__delitem__(key.lower())


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
        self.filters = []
        # Sets a default of a Dummy
        self._dmesg = None
        self.dmesg = False
        self.results_dir = None

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

    def _prepare_test_list(self, opts):
        """ Prepare tests for running

        Flattens the nested group hierarchy into a flat dictionary using '/'
        delimited groups by calling self.flatten_group_hierarchy(), then
        runs it's own filters plus the filters in the self.filters name

        Arguments:
        opts - a core.Options instance

        """
        def matches_any_regexp(x, re_list):
            return any(r.search(x) for r in re_list)

        # The extra argument is needed to match check_all's API
        def test_matches(path, test):
            """Filter for user-specified restrictions"""
            return ((not opts.filter or
                     matches_any_regexp(path, opts.filter)) and
                    path not in opts.exclude_tests and
                    not matches_any_regexp(path, opts.exclude_filter))

        filters = self.filters + [test_matches]

        def check_all(item):
            """ Checks group and test name against all filters """
            path, test = item
            for f in filters:
                if not f(path, test):
                    return False
            return True

        # Filter out unwanted tests
        self.test_list = dict(item for item in self.test_list.iteritems()
                              if check_all(item))

        if not self.test_list:
            print('Error: There are no tests scheduled to run. Aborting run.',
                  file=sys.stderr)
            sys.exit(1)

    def _pre_run_hook(self, opts):
        """ Hook executed at the start of TestProfile.run

        To make use of this hook one will need to subclass TestProfile, and
        set this to do something, as be default it will no-op

        Arguments:
        opts -- a core.Options instance
        """
        pass

    def _post_run_hook(self, opts):
        """ Hook executed at the end of TestProfile.run

        To make use of this hook one will need to subclass TestProfile, and
        set this to do something, as be default it will no-op

        Arguments:
        opts -- a core.Options instance
        """
        pass

    def run(self, opts, logger, backend):
        """ Runs all tests using Thread pool

        When called this method will flatten out self.tests into
        self.test_list, then will prepare a logger, pass opts to the Test
        class, and begin executing tests through it's Thread pools.

        Based on the value of opts.concurrent it will either run all the tests
        concurrently, all serially, or first the thread safe tests then the
        serial tests.

        Finally it will print a final summary of the tests

        Arguments:
        opts -- a core.Options instance
        backend -- a results.Backend derived instance

        """

        self._pre_run_hook(opts)
        Test.OPTS = opts

        chunksize = 1

        self._prepare_test_list(opts)
        log = LogManager(logger, len(self.test_list))

        def test(pair):
            """ Function to call test.execute from .map

            Adds opts which are needed by Test.execute()

            """
            name, test = pair
            test.execute(name, log.get(), self.dmesg)
            backend.write_test(name, test.result)

        def run_threads(pool, testlist):
            """ Open a pool, close it, and join it """
            pool.imap(test, testlist, chunksize)
            pool.close()
            pool.join()

        # Multiprocessing.dummy is a wrapper around Threading that provides a
        # multiprocessing compatible API
        #
        # The default value of pool is the number of virtual processor cores
        single = multiprocessing.dummy.Pool(1)
        multi = multiprocessing.dummy.Pool()

        if opts.concurrent == "all":
            run_threads(multi, self.test_list.iteritems())
        elif opts.concurrent == "none":
            run_threads(single, self.test_list.iteritems())
        else:
            # Filter and return only thread safe tests to the threaded pool
            run_threads(multi, (x for x in self.test_list.iteritems()
                                if x[1].run_concurrent))
            # Filter and return the non thread safe tests to the single pool
            run_threads(single, (x for x in self.test_list.iteritems()
                                 if not x[1].run_concurrent))

        log.get().summary()

        self._post_run_hook(opts)

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
        assert isinstance(group, basestring), type(group)

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
                    assert isinstance(args, basestring)
                    name = args

            assert isinstance(name, basestring)
            lgroup = grouptools.join(group, name)

            self.test_list[lgroup] = test_class(
                args,
                **{k: v for k, v in itertools.chain(default_args.iteritems(),
                                                    kwargs.iteritems())})

        yield adder


def load_test_profile(filename):
    """ Load a python module and return it's profile attribute

    All of the python test files provide a profile attribute which is a
    TestProfile instance. This loads that module and returns it or raises an
    error.

    This method doesn't care about file extensions as a way to be backwards
    compatible with script wrapping piglit. 'tests/quick', 'tests/quick.tests',
    and 'tests/quick.py' are all equally valid for filename

    Arguments:
    filename -- the name of a python module to get a 'profile' from

    """
    mod = importlib.import_module('tests.{0}'.format(
        os.path.splitext(os.path.basename(filename))[0]))

    try:
        return mod.profile
    except AttributeError:
        print("Error: There is not profile attribute in module {0}."
              "Did you specify the right file?".format(filename),
              file=sys.stderr)
        sys.exit(2)


def merge_test_profiles(profiles):
    """ Helper for loading and merging TestProfile instances

    Takes paths to test profiles as arguments and returns a single merged
    TestProfile instance.

    Arguments:
    profiles -- a list of one or more paths to profile files.

    """
    profile = load_test_profile(profiles.pop())
    for p in profiles:
        profile.update(load_test_profile(p))
    return profile
