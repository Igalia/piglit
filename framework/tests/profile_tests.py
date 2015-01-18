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
import platform

import nose.tools as nt
from nose.plugins.skip import SkipTest

import framework.core as core
import framework.dmesg as dmesg
import framework.profile as profile
from framework.tests import utils
from framework import grouptools

# Don't print sys.stderr to the console
sys.stderr = sys.stdout


def test_initialize_testprofile():
    """ TestProfile initializes """
    profile.TestProfile()


@nt.raises(SystemExit)
def test_load_test_profile_no_profile():
    """ Loading a module with no profile name exits

    Beacuse load_test_profile uses test.{} to load a module we need a module in
    tests that doesn't have a profile attribute. The only module that currently
    meets that requirement is __init__.py

    """
    profile.load_test_profile('__init__')


def test_load_test_profile_returns():
    """ load_test_profile returns a TestProfile instance """
    profile_ = profile.load_test_profile('sanity')
    assert isinstance(profile_, profile.TestProfile)


def test_testprofile_default_dmesg():
    """ Dmesg initializes false """
    profile_ = profile.TestProfile()
    assert isinstance(profile_.dmesg, dmesg.DummyDmesg)


def test_testprofile_set_dmesg_true():
    """ Dmesg returns an apropriate dmesg is ste to True """
    if not platform.platform().startswith('Linux'):
        raise SkipTest('No dmesg support on this platform')
    profile_ = profile.TestProfile()
    profile_.dmesg = True
    assert isinstance(profile_.dmesg, dmesg.LinuxDmesg)


def test_testprofile_set_dmesg_false():
    """ Dmesg returns a DummyDmesg if set to False """
    if not platform.platform().startswith('Linux'):
        raise SkipTest('No dmesg support on this platform')
    profile_ = profile.TestProfile()
    profile_.dmesg = True
    profile_.dmesg = False
    assert isinstance(profile_.dmesg, dmesg.DummyDmesg)


def test_testprofile_flatten():
    """ TestProfile.flatten_group_hierarchy flattens and empties self.tests """
    test1 = utils.Test(['thing'])
    test2 = utils.Test(['thing'])
    test3 = utils.Test(['thing'])

    profile_ = profile.TestProfile()
    profile_.tests['group1']['test1'] = test1
    profile_.tests['group1']['group2']['test2'] = test2
    profile_.tests['group1']['group2']['group3']['test3'] = test3

    profile_._flatten_group_hierarchy()

    baseline = profile.Tree({
        'group1/test1': test1,
        'group1/group2/test2': test2,
        'group1/group2/group3/test3': test3,
    })

    # Do not flatten until after baseline, since we want to use the same
    # isntances and tests should be emptied by flattening
    profile_._flatten_group_hierarchy()

    # profile_.tests should have been emptied
    nt.assert_dict_equal(profile_.tests, {})

    nt.assert_dict_equal(profile_.test_list, baseline)


def test_testprofile_update_tests():
    """ TestProfile.update() updates TestProfile.tests

    TestProfile.tests is deprecated, and this test should be removed eventually

    """
    profile1 = profile.TestProfile()
    profile1.tests['group1']['test1'] = utils.Test(['test1'])

    profile2 = profile.TestProfile()
    baseline = profile2.tests
    baseline['group2']['test2'] = utils.Test(['test2'])
    baseline['group1']['test1'] = utils.Test(['test3'])

    profile1.update(profile2)

    nt.assert_dict_equal(profile1.tests, baseline)


def test_testprofile_update_test_list():
    """ TestProfile.update() updates TestProfile.test_list """
    profile1 = profile.TestProfile()
    profile1.test_list['group1/test1'] = utils.Test(['test1'])


    profile2 = profile.TestProfile()
    profile2.test_list['group1/test1'] = utils.Test(['test3'])
    profile2.test_list['group2/test2'] = utils.Test(['test2'])

    profile1.update(profile2)

    nt.assert_dict_equal(profile1.test_list, profile2.test_list)


def generate_prepare_test_list_flatten():
    """ Generate tests for TestProfile.prepare_test_list() """
    test1 = utils.Test(['thingy'])
    test2 = utils.Test(['thingy'])
    test3 = utils.Test(['thingy'])

    tests = profile.Tree()
    tests['group1']['test1'] = test1
    tests['group1']['group3']['test2'] = test2
    tests['group3']['test5'] = test3

    test_list = {
        'group1/test1': test1,
        'group1/group3/test2': test2,
        'group3/test5': test3,
    }

    check_flatten.description = \
        "TestProfile.prepare_test_list flattens TestProfile.tests"
    yield check_flatten, tests, test_list

    check_mixed_flatten.description = \
        "TestProfile flattening is correct when tess and test_list are set"
    yield check_mixed_flatten, tests, test_list


@nt.nottest
def check_flatten(tests, testlist):
    """ TestProfile.prepare_test_list flattens TestProfile.tests """
    profile_ = profile.TestProfile()
    profile_.tests = tests
    profile_._flatten_group_hierarchy()

    nt.assert_dict_equal(profile_.test_list, testlist)


@nt.nottest
def check_mixed_flatten(tests, testlist):
    """ flattening is correct when tests and test_list are defined """
    profile_ = profile.TestProfile()
    profile_.tests = tests
    profile_.test_list['test8'] = utils.Test(['other'])
    profile_._flatten_group_hierarchy()

    baseline = {'test8': profile_.test_list['test8']}
    baseline.update(testlist)

    nt.assert_dict_equal(profile_.test_list, baseline)


class TestPrepareTestListMatches(object):
    """Create tests for TestProfile.prepare_test_list filtering"""
    def __init__(self):
        self.data = {
            'group1/test1': 'thingy',
            'group1/group3/test2': 'thing',
            'group3/test5': 'other',
            'group4/Test9': 'is_caps',
        }

    def test_matches_filter_mar_1(self):
        """TestProfile.prepare_test_list: 'not env.filter or
        matches_any_regex() env.filter is False

        Nothing should be filtered.

        """
        env = core.Options()

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        nt.assert_dict_equal(profile_.test_list, self.data)

    def test_matches_filter_mar_2(self):
        """TestProfile.prepare_test_list: 'not env.filter or matches_any_regex()
        mar is False

        """
        env = core.Options(include_filter=['test5'])

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        baseline = {'group3/test5': 'other'}

        nt.assert_dict_equal(profile_.test_list, baseline)

    def test_matches_env_exclude(self):
        """TestProfile.prepare_test_list: 'not path in env.exclude_tests"""
        env = core.Options()
        env.exclude_tests.add('group3/test5')

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        baseline = copy.deepcopy(self.data)
        del baseline['group3/test5']

        nt.assert_dict_equal(profile_.test_list, baseline)

    def test_matches_exclude_mar(self):
        """TestProfile.prepare_test_list: 'not matches_any_regexp()"""
        env = core.Options(exclude_filter=['test5'])

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        baseline = copy.deepcopy(self.data)
        del baseline['group3/test5']

        nt.assert_dict_equal(profile_.test_list, baseline)

    def test_matches_include_caps(self):
        """TestProfile.prepare_test_list: matches capitalized tests."""
        env = core.Options(exclude_filter=['test9'])

        profile_ = profile.TestProfile()
        profile_.test_list = self.data
        profile_._prepare_test_list(env)

        nt.assert_not_in('group4/Test9', profile_.test_list)


@utils.no_error
def test_testprofile_group_manager_no_name_args_eq_one():
    """TestProfile.group_manager: no name and len(args) == 1 is valid"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a'])


def test_testprofile_group_manager_no_name_args_gt_one():
    """TestProfile.group_manager: no name and len(args) > 1 is valid"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a', 'b'])

    nt.assert_in(grouptools.join('foo', 'a b'), prof.test_list)


@utils.no_error
def test_testprofile_group_manager_name():
    """TestProfile.group_manager: name plus len(args) > 1 is valid"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a', 'b'], 'a')


def test_testprofile_group_manager_is_added():
    """TestProfile.group_manager: Tests are added to the profile"""
    prof = profile.TestProfile()
    with prof.group_manager(utils.Test, 'foo') as g:
        g(['a', 'b'], 'a')

    nt.assert_in(grouptools.join('foo', 'a'), prof.test_list)
