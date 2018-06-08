# encoding=utf-8
# Copyright (c) 2014-2016 Intel Corporation

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

"""Tests for the results module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import pytest
import six

from framework import exceptions
from framework import grouptools
from framework import results
from framework import status

from .backends import shared

# pylint: disable=no-self-use


class TestSubtests(object):
    """Tests for the Subtest class."""

    @pytest.fixture
    def subtest(self):
        return results.Subtests()

    def test_convert_statuses(self, subtest):
        """results.Subtests.__setitem__: converts strings to statues."""
        subtest['foo'] = 'pass'
        assert subtest['foo'] is status.PASS

    def test_to_json(self, subtest):
        """results.Subtests.to_json: sets values properly."""
        baseline = {
            'foo': status.PASS,
            'bar': status.CRASH,
            '__type__': 'Subtests',
        }

        subtest['foo'] = status.PASS
        subtest['bar'] = status.CRASH

        assert baseline == subtest.to_json()

    def test_from_dict(self, subtest):
        """results.Subtests.from_dict: restores values properly"""
        subtest['foo'] = status.PASS
        subtest['bar'] = status.CRASH

        test = results.Subtests.from_dict(subtest.to_json())

        # Subtests don't have equality methods, so convert them to dicts before
        # comparing.
        assert dict(subtest) == dict(test)

    def test_from_dict_instance(self, subtest):
        """results.Subtests.from_dict: restores values properly"""
        subtest['foo'] = status.PASS
        test = results.Subtests.from_dict(subtest.to_json())

        assert test['foo'] is status.PASS


class TestTestResult(object):
    """Tests for the TestResult class."""

    class TestFromDict(object):
        """Tests for TestResult.from_dict."""

        def test_inst(self):
            """results.TestResult.from_dict: returns a TestResult."""
            test = results.TestResult.from_dict({'result': 'pass'})
            assert isinstance(test, results.TestResult)

        class TestAttributes(object):
            """Tests attribute restoration for the from_dict method."""

            @classmethod
            def setup_class(cls):
                """Setup state for all tests."""
                cls.dict = {
                    'returncode': 100,
                    'err': 'this is an err',
                    'out': 'this is some text',
                    'time': {
                        'start': 0.5,
                        'end': 0.9,
                    },
                    'environment': 'some env stuff',
                    'subtests': {
                        'a': 'pass',
                        'b': 'fail',
                    },
                    'result': 'crash',
                    'exception': 'an exception',
                    'dmesg': 'this is dmesg',
                    'pid': [1934],
                }

                cls.test = results.TestResult.from_dict(cls.dict)

            def test_returncode(self):
                """sets returncode properly."""
                assert self.test.returncode == self.dict['returncode']

            def test_err(self):
                """sets err properly."""
                assert self.test.err == self.dict['err']

            def test_out(self):
                """sets out properly."""
                assert self.test.out == self.dict['out']

            def test_time(self):
                """sets time properly.

                Ultimatley time needs to be converted to a TimeAttribute object,
                not a dictionary, however, that functionality is handled by
                backends.json.piglit_decoder, not by the from_dict method of
                TestResult. So in this case the test is that the object is a
                dictionary, not a TimeAttribute because this method shouldn't
                make the coversion.

                """
                # pylint: disable=unsubscriptable-object
                assert self.test.time.start == self.dict['time']['start']
                assert self.test.time.end == self.dict['time']['end']

            def test_environment(self):
                """sets environment properly."""
                assert self.test.environment == self.dict['environment']

            def test_exception(self):
                """sets exception properly."""
                assert self.test.exception == self.dict['exception']

            def test_subtests(self):
                """sets subtests properly."""
                assert self.test.subtests == self.dict['subtests']

            def test_subtests_type(self):
                """subtests are Status instances."""
                assert self.test.subtests['a'] is status.PASS
                assert self.test.subtests['b'] is status.FAIL

            def test_dmesg(self):
                """sets dmesg properly."""
                assert self.test.dmesg == self.dict['dmesg']

            def test_pid(self):
                """sets pid properly."""
                assert self.test.pid == self.dict['pid']

        class TestResult(object):
            """Tests for TestResult.result getter and setter methods."""

            @pytest.fixture
            def result(self):
                return results.TestResult('pass')

            def test_getter_no_subtests(self, result):
                """Getter returns the result when there are no subtests."""
                assert result.result is status.PASS

            def test_getter_with_subtests(self, result):
                """Getter returns worst subtest when subtests are present."""
                result.subtests['a'] = status.PASS
                result.subtests['b'] = status.CRASH

                assert result.result is status.CRASH

            def test_getter_subtests_crash(self, result):
                """Doesn't mask crashes.

                Crash is somewhat unique in that subtests can run and pass, but
                then the test can crash before all subtests have been run.
                """
                result.result = status.CRASH
                result.subtests['a'] = status.PASS
                result.subtests['b'] = status.SKIP

                assert result.result is status.CRASH

            def test_setter(self, result):
                """setter makes the result a status."""
                result.result = status.FAIL
                assert result.result is status.FAIL

            def test_setter_invalid(self, result):
                """setter raises PiglitFatalError for invalid values."""
                with pytest.raises(exceptions.PiglitFatalError):
                    result.result = 'foo'

    class TestToJson(object):
        """Tests for the attributes of the to_json method."""

        @classmethod
        def setup_class(cls):
            """Setup state for all tests."""
            test = results.TestResult()
            test.returncode = 100
            test.err = 'this is an err'
            test.out = 'this is some text'
            test.time.start = 0.3
            test.time.end = 0.5
            test.environment = 'some env stuff'
            test.subtests.update({
                'a': 'pass',
                'b': 'fail'})
            test.result = 'crash'
            test.exception = 'an exception'
            test.dmesg = 'this is dmesg'
            test.pid = 1934
            test.traceback = 'a traceback'

            cls.test = test
            cls.json = test.to_json()

        def test_returncode(self):
            """results.TestResult.to_json: sets the returncode correctly"""
            assert self.test.returncode == self.json['returncode']

        def test_err(self):
            """results.TestResult.to_json: sets the err correctly"""
            assert self.test.err == self.json['err']

        def test_out(self):
            """results.TestResult.to_json: sets the out correctly"""
            assert self.test.out == self.json['out']

        def test_exception(self):
            """results.TestResult.to_json: sets the exception correctly"""
            assert self.test.exception == self.json['exception']

        def test_time(self):
            """results.TestResult.to_json: sets the time correctly"""
            # pylint: disable=unsubscriptable-object
            assert self.test.time.start == self.json['time']['start']
            assert self.test.time.end == self.json['time']['end']

        def test_environment(self):
            """results.TestResult.to_json: sets the environment correctly"""
            assert self.test.environment == self.json['environment']

        def test_subtests(self):
            """results.TestResult.to_json: sets the subtests correctly"""
            assert self.test.subtests['a'] == self.json['subtests']['a']
            assert self.test.subtests['b'] == self.json['subtests']['b']
            assert self.json['subtests']['__type__']

        def test_type(self):
            """results.TestResult.to_json: adds the __type__ hint"""
            assert self.json['__type__'] == 'TestResult'

        def test_dmesg(self):
            """results.TestResult.to_json: Adds the dmesg attribute"""
            assert self.test.dmesg == self.json['dmesg']

        def test_pid(self):
            """results.TestResult.to_json: Adds the pid attribute"""
            assert self.test.pid == self.json['pid']

        def test_traceback(self):
            """results.TestResult.to_json: Adds the traceback attribute"""
            assert self.test.traceback == self.json['traceback']

    class TestUpdate(object):
        """Tests for TestResult.update."""

        def test_no_subtests(self):
            """results.TestResult.update: result is updated"""
            test = results.TestResult('pass')
            test.update({'result': 'incomplete'})
            assert test.result == 'incomplete'

        def test_with_subtests(self):
            """results.TestResult.update: subests are updated"""
            test = results.TestResult('pass')
            test.update({'subtest': {'result': 'incomplete'}})
            assert test.subtests['result'] == 'incomplete'

    class TestTotals(object):
        """Test the totals generated by TestrunResult.calculate_group_totals().
        """
        @classmethod
        def setup_class(cls):
            """setup state for all tests."""
            pass_ = results.TestResult('pass')
            fail = results.TestResult('fail')
            crash = results.TestResult('crash')
            skip = results.TestResult('skip')
            tr = results.TestrunResult()
            tr.tests = {
                'oink': pass_,
                grouptools.join('foo', 'bar'): fail,
                grouptools.join('foo', 'foo', 'bar'): crash,
                grouptools.join('foo', 'foo', 'oink'): skip,
            }

            tr.calculate_group_totals()
            cls.test = tr.totals

        def test_root(self):
            """The root is correct."""
            root = results.Totals()
            root['pass'] += 1
            root['fail'] += 1
            root['crash'] += 1
            root['skip'] += 1

            assert dict(self.test['root']) == dict(root)

        def test_recurse(self):
            """Recurses correctly."""
            expected = results.Totals()
            expected['fail'] += 1
            expected['crash'] += 1
            expected['skip'] += 1
            assert dict(self.test['foo']) == dict(expected)

        def test_two_parents(self):
            """Handles multiple parents correctly."""
            expected = results.Totals()
            expected['crash'] += 1
            expected['skip'] += 1
            assert dict(self.test[grouptools.join('foo', 'foo')]) == \
                dict(expected)

    class TestTotalsWithSubtests(object):
        """results.TestrunResult.totals: Tests with subtests are handled
        correctly.
        """

        @classmethod
        def setup_class(cls):
            """Setup all tests."""
            tr = results.TestResult('crash')
            tr.subtests['foo'] = status.PASS
            tr.subtests['bar'] = status.CRASH
            tr.subtests['oink'] = status.FAIL

            run = results.TestrunResult()
            run.tests[grouptools.join('sub', 'test')] = tr
            run.calculate_group_totals()

            cls.test = run.totals

        def test_root(self):
            """The root is correct with subtests."""
            expect = results.Totals()
            expect['pass'] += 1
            expect['crash'] += 1
            expect['fail'] += 1
            assert dict(self.test['root']) == dict(expect)

        def test_node(self):
            """Tests with subtests are treated as groups."""
            key = grouptools.join('sub', 'test')
            assert key in self.test

        def test_node_values(self):
            """Tests with subtests values are correct."""
            expect = results.Totals()
            expect['pass'] += 1
            expect['crash'] += 1
            expect['fail'] += 1
            assert dict(self.test[grouptools.join('sub', 'test')]) == \
                dict(expect)


class TestStringDescriptor(object):
    """Test class for StringDescriptor."""

    @pytest.fixture
    def test(self):
        class Test(object):
            val = results.StringDescriptor('test')

        return Test()

    def test_get_default(self, test):
        """results.StringDescriptor.__get__: returns default when unset"""
        assert test.val == ''

    def test_set_no_replace(self, test):
        """results.StringDescriptor.__set__: instance is not replaced

        This works by setting the value to a valid value (either bytes or str)
        and then trying to set it to an invalid value (int). If the assignment
        succeeds then the instance has been replaced, if not, it hasn't.

        """
        test.val = 'foo'
        with pytest.raises(TypeError):
            test.val = 1  # pylint: disable=redefined-variable-type

    def test_set_str(self, test):
        """results.StringDescriptor.__set__: str is stored directly"""
        inst = 'foo'
        test.val = inst
        assert test.val == inst

    def test_set_bytes(self, test):
        """results.StringDescriptor.__set__: converts bytes to str"""
        inst = b'foo'
        test.val = inst
        assert test.val == 'foo'

    def test_set_str_unicode_literals(self, test):
        """results.StringDescriptor.__set__: handles unicode litterals in strs.
        """
        test.val = '\ufffd'
        assert test.val == '�'

    def test_delete(self, test):
        """results.StringDescriptor.__delete__: raises NotImplementedError"""
        with pytest.raises(NotImplementedError):
            del test.val


class TestTotals(object):
    """Tests for the totals class."""

    def test_totals_false(self):
        """bool() returns False when all values are 0."""
        assert not bool(results.Totals())

    # The tuple is required because of the timeout status, which conflicts with
    # the timeout pylint plugin
    @pytest.mark.parametrize(
        "key", ((x, ) for x in six.iterkeys(results.Totals())))
    def test_totals_true(self, key):
        """bool() returns True when any value is not 0."""
        test = results.Totals()
        test[key[0]] += 1
        assert bool(test)


class TestTestrunResult(object):
    """Tests for the TestrunResult class."""

    class TestToJson(object):
        """Test TestrunResult.to_json method."""

        @classmethod
        def setup_class(cls):
            """Setup values used by all tests."""
            test = results.TestrunResult()
            test.info = {
                'system': {
                    'uname': 'this is uname',
                    'glxinfo': 'glxinfo',
                    'clinfo': 'clinfo',
                    'wglinfo': 'wglinfo',
                    'lspci': 'this is lspci',
                }
            }
            test.name = 'name'
            test.options = {'some': 'option'}
            test.time_elapsed.end = 1.23
            test.tests = {'a test': results.TestResult('pass')}

            cls.test = test.to_json()

        def test_name(self):
            """name is properly encoded."""
            assert self.test['name'] == 'name'

        def test_info(self):
            assert self.test['info'] == {
                'system': {
                    'uname': 'this is uname',
                    'glxinfo': 'glxinfo',
                    'clinfo': 'clinfo',
                    'wglinfo': 'wglinfo',
                    'lspci': 'this is lspci',
                }
            }

        def test_options(self):
            """options is properly encoded."""
            assert dict(self.test['options']) == {'some': 'option'}

        def test_time(self):
            """time_elapsed is properly encoded."""
            assert self.test['time_elapsed'].end == 1.23

        def test_tests(self):
            """tests is properly encoded."""
            assert self.test['tests']['a test']['result'] == 'pass'

        def test_type(self):
            """__type__ is added."""
            assert self.test['__type__'] == 'TestrunResult'

    class TestFromDict(object):
        """Tests for TestrunResult.from_dict."""

        @pytest.fixture(scope="module")
        def inst(self):
            return results.TestrunResult.from_dict(shared.JSON)

        @pytest.mark.parametrize("attrib", [
            'name', 'results_version', 'info', 'options',
        ])
        def test_attribs_restored(self, attrib, inst):
            """tests for basic attributes."""
            assert shared.JSON[attrib] == getattr(inst, attrib)

        def test_tests(self, inst):
            """tests is restored correctly."""
            assert inst.tests.keys() == shared.JSON['tests'].keys()

        def test_test_type(self, inst):
            """tests is restored correctly."""
            assert isinstance(
                inst.tests['spec@!opengl 1.0@gl-1.0-readpixsanity'],
                results.TestResult)

        def test_totals(self, inst):
            """totals is restored correctly."""
            baseline = shared.JSON['totals'].copy()
            for s in six.itervalues(baseline):
                del s['__type__']
            assert baseline == dict(inst.totals)

        def test_time_elapsed(self, inst):
            """time_elapsed is restored correctly."""
            assert inst.time_elapsed.start == \
                shared.JSON['time_elapsed']['start']
            assert inst.time_elapsed.end == \
                shared.JSON['time_elapsed']['end']

    class TestGetResult(object):
        """Tests for TestrunResult.get_result."""

        @classmethod
        def setup_class(cls):
            """setup state for all tests."""
            tr = results.TestResult('crash')
            tr.subtests['foo'] = status.PASS

            run = results.TestrunResult()
            run.tests['sub'] = tr
            run.tests['test'] = results.TestResult('pass')
            run.calculate_group_totals()

            cls.inst = run

        def test_get_test(self):
            """gets non-subtests."""
            assert self.inst.get_result('test') == 'pass'

        def test_get_subtest(self):
            """gets subtests."""
            assert self.inst.get_result(grouptools.join('sub', 'foo')) == 'pass'

        def test_get_nonexist(self):
            """raises KeyError if test doesn't exist."""
            with pytest.raises(KeyError):
                self.inst.get_result('fooobar')


class TestTimeAttribute(object):
    """Tests for the TimeAttribute class."""

    def test_to_json(self):
        """returns expected dictionary."""
        baseline = {'start': 0.1, 'end': 1.0}
        test = results.TimeAttribute(**baseline)
        baseline['__type__'] = 'TimeAttribute'

        assert baseline == test.to_json()

    def test_from_dict(self):
        """returns expected value."""
        # Type is included because to_json() adds it.
        baseline = {'start': 0.1, 'end': 1.0, '__type__': 'TimeAttribute'}
        test = results.TimeAttribute.from_dict(baseline).to_json()

        assert baseline == test

    def test_total(self):
        """returns the difference between end and start."""
        test = results.TimeAttribute(1.0, 5.0)
        assert test.total == 4.0

    def test_delta(self):
        """results.TimeAttribute.delta: returns the delta of the values"""
        test = results.TimeAttribute(1.0, 5.0)
        assert test.delta == '0:00:04'
