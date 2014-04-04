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

from __future__ import print_function
import os
import sys
import multiprocessing
import multiprocessing.dummy
import importlib

from framework.dmesg import get_dmesg
from framework.log import Log
import framework.exectest

class TestProfile(object):
    def __init__(self):
        self.tests = {}
        self.test_list = {}
        self.filters = []
        # Sets a default of a Dummy
        self.dmesg = False
        self.results_dir = None

    @property
    def dmesg(self):
        """ Return dmesg """
        return self.__dmesg

    @dmesg.setter
    def dmesg(self, not_dummy):
        """ Set dmesg

        Argumnts:
        not_dummy -- if Truthy dmesg will try to get a PosixDmesg, if Falsy it
                     will get a DummyDmesg

        """
        self.__dmesg = get_dmesg(not_dummy)

    def flatten_group_hierarchy(self):
        '''
        Convert Piglit's old hierarchical Group() structure into a flat
        dictionary mapping from fully qualified test names to "Test" objects.

        For example,
        tests['spec']['glsl-1.30']['preprocessor']['compiler']['void.frag']
        would become:
        test_list['spec/glsl-1.30/preprocessor/compiler/void.frag']
        '''

        def f(prefix, group, test_dict):
            for key in group:
                fullkey = key if prefix == '' else os.path.join(prefix, key)
                if isinstance(group[key], dict):
                    f(fullkey, group[key], test_dict)
                else:
                    test_dict[fullkey] = group[key]
        f('', self.tests, self.test_list)
        # Clear out the old Group()
        self.tests = {}

    def prepare_test_list(self, env):
        self.flatten_group_hierarchy()

        def matches_any_regexp(x, re_list):
            return True in map(lambda r: r.search(x) is not None, re_list)

        def test_matches(path, test):
            """Filter for user-specified restrictions"""
            return ((not env.filter or matches_any_regexp(path, env.filter))
                    and not path in env.exclude_tests and
                    not matches_any_regexp(path, env.exclude_filter))

        filters = self.filters + [test_matches]
        def check_all(item):
            path, test = item
            for f in filters:
                if not f(path, test):
                    return False
            return True

        # Filter out unwanted tests
        self.test_list = dict(item for item in self.test_list.iteritems()
                              if check_all(item))

    def pre_run_hook(self):
        """ Hook executed at the start of TestProfile.run

        To make use of this hook one will need to subclass TestProfile, and
        set this to do something, as be dfault it will no-op

        """
        pass

    def post_run_hook(self):
        """ Hook executed at the end of TestProfile.run

        To make use of this hook one will need to subclass TestProfile, and
        set this to do something, as be dfault it will no-op

        """
        pass

    def run(self, env, json_writer):
        '''
        Schedule all tests in profile for execution.

        See ``Test.schedule`` and ``Test.run``.
        '''
        self.prepare_test_list(env)

        self.pre_run_hook()
        framework.exectest.Test.ENV = env

        log = Log(len(self.test_list), env.verbose)

        def test(pair):
            """ Function to call test.execute from .map

            adds env and json_writer which are needed by Test.execute()

            """
            name, test = pair
            test.execute(name, log, json_writer, self.dmesg)

        # Multiprocessing.dummy is a wrapper around Threading that provides a
        # multiprocessing compatible API
        #
        # The default value of pool is the number of virtual processor cores
        single = multiprocessing.dummy.Pool(1)
        multi = multiprocessing.dummy.Pool()
        chunksize = 1

        if env.concurrent == "all":
            multi.imap(test, self.test_list.iteritems(), chunksize)
        elif env.concurrent == "none":
            single.imap(test, self.test_list.iteritems(), chunksize)
        else:
            # Filter and return only thread safe tests to the threaded pool
            multi.imap(test, (x for x in self.test_list.iteritems()
                              if x[1].run_concurrent), chunksize)
            # Filter and return the non thread safe tests to the single pool
            single.imap(test, (x for x in self.test_list.iteritems()
                               if not x[1].run_concurrent), chunksize)

        # Close and join the pools
        # If we don't close and the join the pools the script will exit before
        # the pools finish running
        multi.close()
        single.close()
        multi.join()
        single.join()

        log.summary()

        self.post_run_hook()

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


def loadTestProfile(filename):
    """ Load a python module and return it's profile attribute

    All of the python test files provide a profile attribute which is a
    TestProfile instance. This loads that module and returns it or raises an
    error.

    """
    mod = importlib.import_module('tests.{0}'.format(
        os.path.splitext(os.path.basename(filename))[0]))

    try:
        return mod.profile
    except AttributeError:
        print("Error: There is not profile attribute in module {0}."
              "Did you specify the right file?".format(filename))
        sys.exit(1)


def merge_test_profiles(profiles):
    """ Helper for loading and merging TestProfile instances

    Takes paths to test profiles as arguments and returns a single merged
    TestPRofile instance.

    Arguments:
    profiles -- a list of one or more paths to profile files.

    """
    profile = loadTestProfile(profiles.pop())
    for p in profiles:
        profile.update(loadTestProfile(p))
    return profile
