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

import pytest
import six

from framework import exceptions
from framework import grouptools
from framework import profile
from . import utils

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

    class TestCopy(object):
        """Tests for the copy method."""

        @pytest.fixture
        def fixture(self):
            orig = profile.TestProfile()
            orig.test_list['foo'] = utils.Test(['foo'])
            orig.test_list['bar'] = utils.Test(['bar'])
            orig.filters = [lambda name, _: name != 'bar']
            orig.forced_test_list = ['foo']
            return orig

        def test_filters(self, fixture):
            """The filters attribute is copied correctly."""
            new = fixture.copy()

            # Assert that the fixtures are equivalent, but not the same
            assert fixture.filters == new.filters
            assert fixture.filters is not new.filters

            # And double check by modifying one of them and asserting that the
            # other has not changed.
            new.filters.append(lambda name, _: name != 'oink')
            assert len(fixture.filters) == 1

        def test_forced_test_list(self, fixture):
            """The forced_test_list attribute is copied correctly."""
            new = fixture.copy()

            # Assert that the fixtures are equivalent, but not the same
            assert fixture.forced_test_list == new.forced_test_list
            assert fixture.forced_test_list is not new.forced_test_list

            # And double check by modifying one of them and asserting that the
            # other has not changed.
            del new.forced_test_list[0]
            assert fixture.forced_test_list[0] == 'foo'

        def test_test_list(self, fixture):
            """The test_list attribute is copied correctly."""
            new = fixture.copy()

            # Assert that the fixtures are equivalent, but not the same
            assert fixture.test_list == new.test_list
            assert fixture.test_list is not new.test_list


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

        def test_with_groupmanager(self, test):
            """profile.TestProfile: allow_reassignment wrapper works with
            groupmanager.
            """
            testname = grouptools.join('a', 'b')
            test[testname] = utils.Test(['foo'])
            with test.allow_reassignment:
                with test.group_manager(utils.Test, 'a') as g:
                    g(['bar'], 'b')

            assert test[testname].command == ['bar']

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

    class TestGroupManager(object):
        """Test the group_manager method."""

        @pytest.fixture
        def inst(self):
            return profile.TestDict()

        def test_no_name_args_eq_one(self, inst):
            """no name and len(args) == 1 is valid."""
            with inst.group_manager(utils.Test, 'foo') as g:
                g(['a'])

            assert grouptools.join('foo', 'a') in inst

        def test_no_name_args_gt_one(self, inst):
            """no name and len(args) > 1 is valid."""
            with inst.group_manager(utils.Test, 'foo') as g:
                g(['a', 'b'])

            assert grouptools.join('foo', 'a b') in inst

        def test_name(self, inst):
            """name plus len(args) > 1 is valid."""
            with inst.group_manager(utils.Test, 'foo') as g:
                g(['a', 'b'], 'a')

            assert grouptools.join('foo', 'a') in inst

        def test_adder_kwargs_passed(self, inst):
            """Extra kwargs passed to the adder function are passed to the
            Test.
            """
            with inst.group_manager(utils.Test, 'foo') as g:
                g(['a'], run_concurrent=True)
            test = inst[grouptools.join('foo', 'a')]

            assert test.run_concurrent is True

        def test_context_manager_keyword_args_passed(self, inst):
            """kwargs passed to the context_manager are passed to the Test."""
            with inst.group_manager(
                    utils.Test, 'foo', run_concurrent=True) as g:  # pylint: disable=bad-continuation
                    # This is a pylint bug, there's an upstream report
                g(['a'])
            test = inst[grouptools.join('foo', 'a')]

            assert test.run_concurrent is True

        def test_adder_kwargs_overwrite_context_manager_kwargs(self, inst):
            """default_args are overwritten by kwargs."""
            with inst.group_manager(
                    utils.Test, 'foo', run_concurrent=True) as g:  # pylint: disable=bad-continuation
                    # This is a pylint bug, there's an upstream report
                g(['a'], run_concurrent=False)

            test = inst[grouptools.join('foo', 'a')]
            assert test.run_concurrent is False


class TestRegexFilter(object):
    """Tests for the RegexFilter class."""

    class TestNormal(object):
        """Tests for inverse set to False (default)."""

        def test_empty(self):
            """Returns True when no filters are provided."""
            test = profile.RegexFilter([])
            assert test('foobob', None)

        def test_matches(self):
            """Returns True when the test matches any regex."""
            test = profile.RegexFilter([r'foo', r'bar'])
            assert test('foobob', None)

        def test_not_matches(self):
            """Returns True when the test matches any regex."""
            test = profile.RegexFilter([r'fob', r'bar'])
            assert not test('foobob', None)

    class TestInverse(object):
        """Tests for inverse set to True."""

        def test_empty(self):
            """Returns True when no filters are provided."""
            test = profile.RegexFilter([], inverse=True)
            assert test('foobob', None)

        def test_matches(self):
            """Returns False when the test matches any regex."""
            test = profile.RegexFilter([r'foo', r'bar'], inverse=True)
            assert not test('foobob', None)

        def test_not_matches(self):
            """Returns False when the test matches any regex."""
            test = profile.RegexFilter([r'fob', r'bar'], inverse=True)
            assert test('foobob', None)
