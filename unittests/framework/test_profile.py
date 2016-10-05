# Copyright (c) 2014, 2016 Intel Corporation

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

""" Provides test for the framework.profile modules """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import copy
try:
    from unittest import mock
except ImportError:
    import mock

import pytest
import six

from framework import dmesg
from framework import exceptions
from framework import grouptools
from framework import options
from framework import profile
from framework.test.gleantest import GleanTest
from . import utils
from . import skip

# pylint: disable=invalid-name,no-self-use,protected-access


class TestLoadTestProfile(object):
    """Tests for profile.load_test_profile."""

    def test_no_profile_attribute(self, tmpdir):
        """profile.load_test_profile: Loading a module with no profile name
        exits.

        Because load_test_profile uses test.{} to load a module we need a
        module in tests that doesn't have a profile attribute. The only module
        that currently meets that requirement is __init__.py
        """
        p = tmpdir.join('foo.profile')

        with pytest.raises(exceptions.PiglitFatalError):
            profile.load_test_profile(six.text_type(p))

    def test_no_such_module(self, tmpdir):
        """profile.load_test_profile: Trying to load a non-existent module
        exits.
        """
        # Ensure that the module will not exist by moving to an empty temporary
        # directory
        tmpdir.chdir()
        with pytest.raises(exceptions.PiglitFatalError):
            profile.load_test_profile('this_module_will_never_ever_exist')

    def test_return_type(self):
        """profile.load_test_profile: returns a TestProfile instance."""
        assert isinstance(profile.load_test_profile('sanity'),
                          profile.TestProfile)


class TestTestProfile(object):
    """Tests for profile.TestProfile."""

    def test_default_dmesg(self):
        """profile.TestProfile: Dmesg is dummy by default."""
        profile_ = profile.TestProfile()
        assert isinstance(profile_.dmesg, dmesg.DummyDmesg)

    @skip.linux
    def test_set_dmesg_true(self):
        """profile.TestProfile: Dmesg returns an appropriate dmesg is set to
        True.
        """
        profile_ = profile.TestProfile()
        profile_.dmesg = True
        assert isinstance(profile_.dmesg, dmesg.LinuxDmesg)

    @skip.linux
    def test_set_dmesg_false(self):
        """profile.TestProfile: Dmesg returns a DummyDmesg if set to False."""
        profile_ = profile.TestProfile()
        profile_.dmesg = True
        profile_.dmesg = False
        assert isinstance(profile_.dmesg, dmesg.DummyDmesg)

    def test_update_test_list(self):
        """profile.TestProfile.update(): updates TestProfile.test_list"""
        profile1 = profile.TestProfile()
        group1 = grouptools.join('group1', 'test1')
        group2 = grouptools.join('group1', 'test2')

        profile1.test_list[group1] = utils.Test(['test1'])

        profile2 = profile.TestProfile()
        profile2.test_list[group1] = utils.Test(['test3'])
        profile2.test_list[group2] = utils.Test(['test2'])

        with profile1.allow_reassignment:
            profile1.update(profile2)

        assert dict(profile1.test_list) == dict(profile2.test_list)

    class TestPrepareTestList(object):
        """Create tests for TestProfile.prepare_test_list filtering."""

        @classmethod
        def setup_class(cls):
            cls.opts = None
            cls.data = None
            cls.__patcher = mock.patch('framework.profile.options.OPTIONS',
                                       new_callable=options._Options)

        def setup(self):
            """Setup each test."""
            self.data = profile.TestDict()
            self.data[grouptools.join('group1', 'test1')] = \
                utils.Test(['thingy'])
            self.data[grouptools.join('group1', 'group3', 'test2')] = \
                utils.Test(['thing'])
            self.data[grouptools.join('group3', 'test5')] = \
                utils.Test(['other'])
            self.data[grouptools.join('group4', 'Test9')] = \
                utils.Test(['is_caps'])
            self.opts = self.__patcher.start()

        def teardown(self):
            self.__patcher.stop()

        def test_matches_filter_mar_1(self):
            """profile.TestProfile.prepare_test_list: 'not env.filter or
            matches_any_regex()' env.filter is False.

            Nothing should be filtered.
            """
            profile_ = profile.TestProfile()
            profile_.test_list = self.data
            profile_.prepare_test_list()

            assert dict(profile_.test_list) == dict(self.data)

        def test_matches_filter_mar_2(self):
            """profile.TestProfile.prepare_test_list: 'not env.filter or
            matches_any_regex()' mar is False.
            """
            self.opts.include_filter = ['test5']

            profile_ = profile.TestProfile()
            profile_.test_list = self.data
            profile_.prepare_test_list()

            baseline = {
                grouptools.join('group3', 'test5'): utils.Test(['other'])}

            assert dict(profile_.test_list) == baseline

        def test_matches_env_exclude(self):
            """profile.TestProfile.prepare_test_list: 'not path in
            env.exclude_tests'.
            """
            # Pylint can't figure out that self.opts isn't a dict, but an
            # options._Option object.
            self.opts.exclude_tests.add(grouptools.join('group3', 'test5'))  # pylint: disable=no-member

            baseline = copy.deepcopy(self.data)
            del baseline[grouptools.join('group3', 'test5')]

            profile_ = profile.TestProfile()
            profile_.test_list = self.data
            profile_.prepare_test_list()

            assert dict(profile_.test_list) == dict(baseline)

        def test_matches_exclude_mar(self):
            """profile.TestProfile.prepare_test_list: 'not
            matches_any_regexp()'.
            """
            self.opts.exclude_filter = ['test5']

            baseline = copy.deepcopy(self.data)
            del baseline[grouptools.join('group3', 'test5')]

            profile_ = profile.TestProfile()
            profile_.test_list = self.data
            profile_.prepare_test_list()

            assert dict(profile_.test_list) == dict(baseline)

        def test_matches_include_caps(self):
            """profile.TestProfile.prepare_test_list: matches capitalized
            tests.
            """
            self.opts.exclude_filter = ['test9']

            profile_ = profile.TestProfile()
            profile_.test_list = self.data
            profile_.prepare_test_list()

            assert grouptools.join('group4', 'Test9') not in profile_.test_list

    class TestGroupManager(object):
        """Tests for TestProfile.group_manager."""

        profile = None

        def setup(self):
            self.profile = profile.TestProfile()

        def test_no_name_args_eq_one(self):
            """no name and len(args) == 1 is valid."""
            with self.profile.group_manager(utils.Test, 'foo') as g:
                g(['a'])

            assert grouptools.join('foo', 'a') in self.profile.test_list

        def test_no_name_args_gt_one(self):
            """no name and len(args) > 1 is valid."""
            with self.profile.group_manager(utils.Test, 'foo') as g:
                g(['a', 'b'])

            assert grouptools.join('foo', 'a b') in self.profile.test_list

        def test_name(self):
            """name plus len(args) > 1 is valid."""
            with self.profile.group_manager(utils.Test, 'foo') as g:
                g(['a', 'b'], 'a')

            assert grouptools.join('foo', 'a') in self.profile.test_list

        def test_adder_kwargs_passed(self):
            """Extra kwargs passed to the adder function are passed to the
            Test.
            """
            with self.profile.group_manager(utils.Test, 'foo') as g:
                g(['a'], run_concurrent=True)
            test = self.profile.test_list[grouptools.join('foo', 'a')]

            assert test.run_concurrent is True

        def test_context_manager_keyword_args_passed(self):
            """kwargs passed to the context_manager are passed to the Test."""
            with self.profile.group_manager(
                    utils.Test, 'foo', run_concurrent=True) as g:  # pylint: disable=bad-continuation
                    # This is a pylint bug, there's an upstream report
                g(['a'])
            test = self.profile.test_list[grouptools.join('foo', 'a')]

            assert test.run_concurrent is True

        def test_adder_kwargs_overwrite_context_manager_kwargs(self):
            """default_args are overwritten by kwargs."""
            with self.profile.group_manager(
                    utils.Test, 'foo', run_concurrent=True) as g:  # pylint: disable=bad-continuation
                    # This is a pylint bug, there's an upstream report
                g(['a'], run_concurrent=False)

            test = self.profile.test_list[grouptools.join('foo', 'a')]
            assert test.run_concurrent is False

        def test_name_as_str(self):
            """if args is a string it is not joined.

            This is a "feature" of glean, and no longer needs to be protected
            whenever glean dies.
            """
            with self.profile.group_manager(GleanTest, 'foo') as g:
                g('abc')

            assert grouptools.join('foo', 'abc') in self.profile.test_list


class TestTestDict(object):
    """Tests for the TestDict object."""

    test = None

    def setup(self):
        self.test = profile.TestDict()

    @pytest.mark.parametrize(
        "arg",
        [None, utils.Test(['foo']), ['a'], {'a': 1}],
        ids=six.text_type)
    def test_key_not_string(self, arg):
        """profile.TestDict: If key value isn't a string an exception is raised.

        This throws a few different things at the key value and expects an
        error to be raised. It isn't a perfect test, but it was the best I
        could come up with.
        """
        with pytest.raises(exceptions.PiglitFatalError):
            self.test[arg] = utils.Test(['foo'])

    def test_reassignment(self):
        """reassigning a key raises an exception."""
        self.test['foo'] = utils.Test(['foo'])
        with pytest.raises(exceptions.PiglitFatalError):
            self.test['foo'] = utils.Test(['foo', 'bar'])

    class TestAllowReassignment(object):
        """Tests for TestDict.allow_reassignment."""

        @pytest.fixture
        def test(self):
            return profile.TestDict()

        @pytest.fixture
        def prof(self):
            return profile.TestProfile()

        def test_case_insensitve(self, test):
            """reassigning a key raises an exception (capitalization is
            ignored)."""
            test['foo'] = utils.Test(['foo'])
            with pytest.raises(exceptions.PiglitFatalError):
                test['Foo'] = utils.Test(['foo', 'bar'])

        def test_simple(self, test):
            """profile.TestDict.allow_reassignment works."""
            test['a'] = utils.Test(['foo'])
            with test.allow_reassignment:
                test['a'] = utils.Test(['bar'])

            assert test['a'].command == ['bar']

        def test_with_groupmanager(self, prof):
            """profile.TestProfile: allow_reassignment wrapper works with
            groupmanager.
            """
            testname = grouptools.join('a', 'b')
            prof.test_list[testname] = utils.Test(['foo'])
            with prof.allow_reassignment:
                with prof.group_manager(utils.Test, 'a') as g:
                    g(['bar'], 'b')

            assert prof.test_list[testname].command == ['bar']

        def test_stacked(self, test):
            """profile.profile.TestDict.allow_reassignment: check stacking
            cornercase.

            There is an odd corner case in the original (obvious)
            implementation of this function, If one opens two context managers
            and then returns from the inner one assignment will not be allowed,
            even though one is still inside the first context manager.
            """
            test['a'] = utils.Test(['foo'])
            with test.allow_reassignment:
                with test.allow_reassignment:
                    pass
                test['a'] = utils.Test(['bar'])

            assert test['a'].command == ['bar']
