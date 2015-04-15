# Copyright (c) 2014 Intel Corporation

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

from __future__ import print_function, absolute_import
import sys
import copy

import nose.tools as nt

import framework.core as core
import framework.dmesg as dmesg
import framework.profile as profile
from framework.tests import utils
from framework import grouptools
from framework.test import GleanTest

# Don't print sys.stderr to the console
sys.stderr = sys.stdout


@utils.no_error
def test_initialize_testprofile():
    """profile.TestProfile: class initializes"""
    profile.TestProfile()


@nt.raises(SystemExit)
def test_load_test_profile_no_profile():
    """profile.load_test_profile: Loading a module with no profile name exits

    Beacuse load_test_profile uses test.{} to load a module we need a module in
    tests that doesn't have a profile attribute. The only module that currently
    meets that requirement is __init__.py

    """
    profile.load_test_profile('__init__')


def test_load_test_profile_returns():
    """profile.load_test_profile: returns a TestProfile instance"""
    profile_ = profile.load_test_profile('sanity')
    assert isinstance(profile_, profile.TestProfile)


def test_testprofile_default_dmesg():
    """profile.TestProfile: Dmesg initializes False"""
    profile_ = profile.TestProfile()
    assert isinstance(profile_.dmesg, dmesg.DummyDmesg)


def test_testprofile_set_dmesg_true():
    """profile.TestProfile: Dmesg returns an apropriate dmesg is ste to True"""
    utils.platform_check('linux')
    profile_ = profile.TestProfile()
    profile_.dmesg = True
    assert isinstance(profile_.dmesg, dmesg.LinuxDmesg)


def test_testprofile_set_dmesg_false():
    """profile.TestProfile: Dmesg returns a DummyDmesg if set to False"""
    utils.platform_check('linux')
    profile_ = profile.TestProfile()
    profile_.dmesg = True
    profile_.dmesg = False
    assert isinstance(profile_.dmesg, dmesg.DummyDmesg)


def test_testprofile_update_test_list():
    """profile.TestProfile.update(): updates TestProfile.test_list"""
    profile1 = profile.TestProfile()
    group1 = grouptools.join('group1', 'test1')
    group2 = grouptools.join('group1', 'test2')

    profile1.test_list[group1] = utils.Test(['test1'])


    profile2 = profile.TestProfile()
    profile2.test_list[group1] = utils.Test(['test3'])
    profile2.test_list[group2] = utils.Test(['test2'])

    profile1.update(profile2)

    nt.assert_dict_equal(profile1.test_list, profile2.test_list)


class TestPrepareTestListMatches(object):
    """Create tests for TestProfile.prepare_test_list filtering"""
    def __init__(self):
        self.data = {
            grouptools.join('group1', 'test1'): 'thingy',
            grouptools.join('group1', 'group3', 'test2'): 'thing',
            grouptools.join('group3', 'test5'): 'other',
            grouptools.join('group4', 'Test9'): 'is_caps',
        }

    def test_matches_filter_mar_1(self):
        """profile.TestProfile.prepare_test_list: 'not env.filter or matches_any_regex()' env.filter is False

        Nothing should be filtered.

        """
        env = core.Options()

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        nt.assert_dict_equal(profile_.test_list, self.data)

    def test_matches_filter_mar_2(self):
        """profile.TestProfile.prepare_test_list: 'not env.filter or matches_any_regex()' mar is False"""
        env = core.Options(include_filter=['test5'])

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        baseline = {grouptools.join('group3', 'test5'): 'other'}

        nt.assert_dict_equal(profile_.test_list, baseline)

    def test_matches_env_exclude(self):
        """profile.TestProfile.prepare_test_list: 'not path in env.exclude_tests'"""
        env = core.Options()
        env.exclude_tests.add(grouptools.join('group3', 'test5'))

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        baseline = copy.deepcopy(self.data)
        del baseline[grouptools.join('group3', 'test5')]

        nt.assert_dict_equal(profile_.test_list, baseline)

    def test_matches_exclude_mar(self):
        """profile.TestProfile.prepare_test_list: 'not matches_any_regexp()'"""
        env = core.Options(exclude_filter=['test5'])

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        baseline = copy.deepcopy(self.data)
        del baseline[grouptools.join('group3', 'test5')]

        nt.assert_dict_equal(profile_.test_list, baseline)

    def test_matches_include_caps(self):
        """profile.TestProfile.prepare_test_list: matches capitalized tests"""
        env = core.Options(exclude_filter=['test9'])

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        nt.assert_not_in(grouptools.join('group4', 'Test9'), profile_.test_list)


@utils.no_error
def test_testprofile_group_manager_no_name_args_eq_one():
    """profile.TestProfile.group_manager: no name and len(args) == 1 is valid"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a'])


def test_testprofile_group_manager_no_name_args_gt_one():
    """profile.TestProfile.group_manager: no name and len(args) > 1 is valid"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a', 'b'])

    nt.assert_in(grouptools.join('foo', 'a b'), prof.test_list)


@utils.no_error
def test_testprofile_group_manager_name():
    """profile.TestProfile.group_manager: name plus len(args) > 1 is valid"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a', 'b'], 'a')


def test_testprofile_group_manager_is_added():
    """profile.TestProfile.group_manager: Tests are added to the profile"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a', 'b'], 'a')

    nt.assert_in(grouptools.join('foo', 'a'), prof.test_list)


def test_testprofile_groupmanager_kwargs():
    """profile.TestProfile.group_manager: Extra kwargs are passed to the Test"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a'], run_concurrent=True)

    test = prof.test_list[grouptools.join('foo', 'a')]
    nt.assert_equal(test.run_concurrent, True)


def test_testprofile_groupmanager_default_args():
    """profile.TestProfile.group_manager: group_manater kwargs are passed to the Test"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo', run_concurrent=True) as g:
        g(['a'])

    test = prof.test_list[grouptools.join('foo', 'a')]
    nt.assert_equal(test.run_concurrent, True)


def test_testprofile_groupmanager_kwargs_overwrite():
    """profile.TestProfile.group_manager: default_args are overwritten by kwargs"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo', run_concurrent=True) as g:
        g(['a'], run_concurrent=False)

    test = prof.test_list[grouptools.join('foo', 'a')]
    nt.assert_equal(test.run_concurrent, False)


def test_testprofile_groupmanager_name_str():
    """profile.TestProfile.group_manager: if args is a string it is not joined"""
    prof = profile.TestProfile()
    # Yes, this is really about supporting gleantest anyway.
    with prof.group_manager(GleanTest, 'foo') as g:
        g('abc')

    nt.ok_(grouptools.join('foo', 'abc') in prof.test_list)


@nt.raises(profile.TestDictError)
def test_testdict_key_not_string():
    """profile.TestDict: If key value isn't a string an exception is raised.

    This throws a few different things at the key value and expects an error to
    be raised. It isn't a perfect test, but it was the best I could come up
    with.

    """
    test = profile.TestDict()

    for x in [None, utils.Test(['foo']), ['a'], {'a': 1}]:
        test[x] = utils.Test(['foo'])


@nt.raises(profile.TestDictError)
def test_testdict_value_not_valid():
    """profile.TestDict: If the value isn't a Test an exception is raised.

    Like the key test this isn't perfect, but it does try the common mistakes.

    """
    test = profile.TestDict()

    for x in [{}, 'a']:
        test['foo'] = x


@nt.raises(profile.TestDictError)
def test_testdict_reassignment():
    """profile.TestDict: reassigning a key raises an exception"""
    test = profile.TestDict()
    test['foo'] = utils.Test(['foo'])
    test['foo'] = utils.Test(['foo', 'bar'])


@nt.raises(profile.TestDictError)
def test_testdict_reassignment_lower():
    """profile.TestDict: reassigning a key raises an exception (capitalization is ignored)"""
    test = profile.TestDict()
    test['foo'] = utils.Test(['foo'])
    test['Foo'] = utils.Test(['foo', 'bar'])


def test_testdict_allow_reassignment():
    """profile.TestDict: allow_reassignment works"""
    test = profile.TestDict()
    test['a'] = utils.Test(['foo'])
    with test.allow_reassignment:
        test['a'] = utils.Test(['bar'])

    nt.ok_(test['a'].command == ['bar'])


def test_testprofile_allow_reassignment():
    """profile.TestProfile: allow_reassignment wrapper works"""
    prof = profile.TestProfile()
    prof.test_list['a'] = utils.Test(['foo'])
    with prof.allow_reassignment:
        prof.test_list['a'] = utils.Test(['bar'])

    nt.ok_(prof.test_list['a'].command == ['bar'])


def test_testprofile_allow_reassignment_with_groupmanager():
    """profile.TestProfile: allow_reassignment wrapper works with groupmanager"""
    testname = grouptools.join('a', 'b')
    prof = profile.TestProfile()
    prof.test_list[testname] = utils.Test(['foo'])
    with prof.allow_reassignment:
        with prof.group_manager(utils.Test, 'a') as g:
            g(['bar'], 'b')

    nt.ok_(prof.test_list[testname].command == ['bar'])


def test_testprofile_allow_reassignemnt_stacked():
    """profile.profile.TestDict.allow_reassignment: check stacking cornercase

    There is an odd corner case in the original (obvious) implmentation of this
    function, If one opens two context managers and then returns from the inner
    one assignment will not be allowed, even though one is still inside the
    first context manager.

    """
    test = profile.TestDict()
    test['a'] = utils.Test(['foo'])
    with test.allow_reassignment:
        with test.allow_reassignment:
            pass
        test['a'] = utils.Test(['bar'])

    nt.ok_(test['a'].command == ['bar'])
