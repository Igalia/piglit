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
        assert isinstance(value, (Tree, Test, types.NoneType)), \
               "Values must be either a Tree or a Test, but was {}".format(
                   type(value))

        super(TestDict, self).__setitem__(key.lower(), value)

    def __getitem__(self, key):
        """Lower the value before returning."""
        return super(TestDict, self).__getitem__(key.lower())


class Tree(TestDict):  # pylint: disable=too-few-public-methods
    """A tree-like object built with python dictionaries.

    When a node that doesn't exist is requested it is automatically created
    with a new Tree node.

    This also enforces lowering of keys, both for getting and setting.

    """
    def __missing__(self, key):
        """Automatically create new Tree nodes."""
        self[key] = Tree()
        return self[key]


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
        # Self.tests is deprecated, see above
        self.tests = Tree()
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

    def _flatten_group_hierarchy(self):
        """ Flatten nested dictionary structure

        Convert Piglit's old hierarchical Group() structure into a flat
        dictionary mapping from fully qualified test names to "Test" objects.

        For example,
        self.tests['spec']['glsl-1.30']['preprocessor']['compiler']['void.frag']
        would become:
        self.test_list['spec/glsl-1.30/preprocessor/compiler/void.frag']

        """

        def f(prefix, group, test_dict):
            """ Recursively flatter nested dictionary tree """
            for key, value in group.iteritems():
                fullkey = grouptools.join(prefix, key)
                if isinstance(value, Tree):
                    f(fullkey, value, test_dict)
                else:
                    test_dict[fullkey] = value
        f('', self.tests, self.test_list)

        # Empty the nested structure. Do not use clear(), since it will
        # actually remove all of the objects in the Tree, which will also
        # remove them from self.test_list
        self.tests = Tree()

    def _prepare_test_list(self, opts):
        """ Prepare tests for running

        Flattens the nested group hierarchy into a flat dictionary using '/'
        delimited groups by calling self.flatten_group_hierarchy(), then
        runs it's own filters plus the filters in the self.filters name

        Arguments:
        opts - a core.Options instance

        """
        self._flatten_group_hierarchy()

        def matches_any_regexp(x, re_list):
            return any(r.search(x) for r in re_list)

        # The extra argument is needed to match check_all's API
        def test_matches(path, test):
            """Filter for user-specified restrictions"""
            return ((not opts.filter or matches_any_regexp(path, opts.filter))
                    and not path in opts.exclude_tests and
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

        self._pre_run_hook()
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

        self._post_run_hook()

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
            self.tests.update(profile.tests)
            self.test_list.update(profile.test_list)


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
