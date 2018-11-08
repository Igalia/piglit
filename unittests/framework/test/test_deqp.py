# coding=utf-8
# Copyright (c) 2015-2016 Intel Corporation

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

"""Tests for the dEQP integration in framework.

This tests the core framework shared code, and not the individual packages in
tests

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import textwrap
try:
    from unittest import mock
except ImportError:
    import mock

import pytest
import six

from framework import exceptions
from framework import grouptools
from framework import profile
from framework import status
from framework.test import deqp

# pylint:disable=invalid-name,no-self-use


class _DEQPTestTest(deqp.DEQPBaseTest):
    deqp_bin = 'deqp.bin'
    extra_args = ['extra']


class TestGetOptions(object):
    """Tests for the get_option function."""

    @pytest.fixture
    def env(self, mocker):
        """Create a mocked os.environ."""
        return mocker.patch('framework.test.deqp.os.environ', {})

    @pytest.fixture
    def conf(self, mocker):
        """Create a mocked piglit.conf."""
        return mocker.patch('framework.core.PIGLIT_CONFIG.safe_get',
                            mocker.Mock(return_value=None))

    def test_from_default(self):
        """deqp.get_option: if env is set it overrides piglit.conf."""
        # The mock means that only the first value matters
        actual = deqp.get_option('foo', ('foo', 'foo'),
                                 default=mock.sentinel.good)

        assert actual is mock.sentinel.good

    def test_from_conf(self, conf):
        """deqp.get_option: if env is not set a value is taken from
        piglit.conf.
        """
        conf.return_value = mock.sentinel

        # The mock means that these values don't actually matter
        actual = deqp.get_option('foo', ('foo', 'foo'))

        assert actual is mock.sentinel

    def test_from_env(self, env, conf):
        """deqp.get_option: if env is set it overrides piglit.conf."""
        conf.return_value = mock.sentinel.bad
        env['TEST'] = mock.sentinel.good

        # The mock means that only the first value matters
        actual = deqp.get_option('TEST', ('foo', 'foo'))

        assert actual is mock.sentinel.good

    def test_get_option_required(self, mocker):
        """deqp.get_option: dies if a required option cannot be retrieved."""
        mocker.patch('framework.test.deqp.os.environ', {}, True)

        with pytest.raises(exceptions.PiglitFatalError):
            deqp.get_option('NOT_REAL', ('fake', 'fake'), default='',
                            required=True)


class TestMakeProfile(object):
    """Test deqp.make_profile."""

    @classmethod
    def setup_class(cls):
        cls.profile = deqp.make_profile(['this.is.a.deqp.test'], _DEQPTestTest)

    def test_returns_profile(self):
        """deqp.make_profile: returns a TestProfile."""
        assert isinstance(self.profile, profile.TestProfile)

    def test_replaces_separator(self):
        """deqp.make_profile: replaces '.' with grouptools.separator"""
        expected = grouptools.join('this', 'is', 'a', 'deqp', 'test')
        assert expected in self.profile.test_list


class TestIterDeqpTestCases(object):
    """Tests for iter_deqp_test_cases."""

    def _do_test(self, write, expect, tmpdir):
        """Run the acutal test."""
        p = tmpdir.join('foo')
        p.write(write)
        gen = deqp.iter_deqp_test_cases(six.text_type(p))
        assert next(gen) == expect

    def test_test_cases(self, tmpdir):
        """Correctly detects a test line."""
        self._do_test('TEST: a.deqp.test', 'a.deqp.test', tmpdir)

    def test_test_group(self, tmpdir):
        """Correctly detects a group line."""
        self._do_test('GROUP: a group\nTEST: a.deqp.test', 'a.deqp.test',
                      tmpdir)

    def test_bad_entry(self, tmpdir):
        """A PiglitFatalException is raised if a line is not a TEST or GROUP.
        """
        with pytest.raises(exceptions.PiglitFatalError):
            self._do_test('this will fail', None, tmpdir)


class TestDEQPBaseTest(object):
    """Test the DEQPBaseTest class."""

    @classmethod
    def setup_class(cls):
        cls.test = _DEQPTestTest('a.deqp.test')

    def test_command_adds_extra_args(self):
        assert self.test.command[-1] == 'extra'

    class TestInterpretResultReturncodes(object):
        """Test the interpret_result method's returncode handling."""

        @classmethod
        def setup_class(cls):
            cls.test = _DEQPTestTest('a.deqp.test')

        def test_crash(self):
            """deqp.DEQPBaseTest.interpret_result: if returncode is < 0 stauts
            is crash.
            """
            self.test.result.returncode = -9
            self.test.interpret_result()
            assert self.test.result.result is status.CRASH

        def test_returncode_fail(self):
            """deqp.DEQPBaseTest.interpret_result: if returncode is > 0 result
            is fail.
            """
            self.test.result.returncode = 1
            self.test.interpret_result()
            assert self.test.result.result is status.FAIL

        def test_fallthrough(self):
            """deqp.DEQPBaseTest.interpret_result: if no case is hit set to
            fail.
            """
            self.test.result.returncode = 0
            self.test.result.out = ''
            self.test.interpret_result()
            assert self.test.result.result is status.FAIL

        def test_windows_returncode_3(self, mocker):
            """deqp.DEQPBaseTest.interpret_result: on windows returncode 3 is
            crash.
            """
            mocker.patch('framework.test.base.sys.platform', 'win32')
            self.test.result.returncode = 3
            self.test.interpret_result()
            assert self.test.result.result is status.CRASH

    class TestDEQPBaseTestIntepretResultOutput(object):
        """Tests for DEQPBaseTest.__find_map."""

        inst = None
        __OUT = textwrap.dedent("""\
            dEQP Core 2014.x (0xcafebabe) starting..
              target implementation = 'DRM'

            Test case 'dEQP-GLES2.functional.shaders.conversions.vector_to_vector.vec3_to_ivec3_fragment'..
            Vertex shader compile time = 0.129000 ms
            Fragment shader compile time = 0.264000 ms
            Link time = 0.814000 ms
            Test case duration in microseconds = 487155 us
              {stat} ({stat})

            DONE!

            Test run totals:
              Passed:        {pass_}/1 (100.0%)
              Failed:        {fail}/1 (0.0%)
              Not supported: {supp}/1 (0.0%)
              Warnings:      {warn}/1 (0.0%)
            Test run was ABORTED!
        """)

        def __gen_stdout(self, stat):
            """Make a string that looks like DEQP output."""
            assert stat in ['Fail', 'NotSupported', 'Pass', 'QualityWarning',
                            'InternalError', 'Crash', 'ResourceError']

            return self.__OUT.format(
                stat=stat,
                pass_=1 if stat == 'Pass' else 0,
                fail=1 if stat in ['Crash', 'Fail', 'ResourceError'] else 0,
                supp=1 if stat == 'InternalError' else 0,
                warn=1 if stat == 'QualityWarning' else 0,
            )

        def setup(self):
            self.inst = _DEQPTestTest('a.deqp.test')
            self.inst.result.returncode = 0

        def test_fail(self):
            """test.deqp.DEQPBaseTest.interpret_result: when Fail in result the
            result is 'fail'.
            """
            self.inst.result.out = self.__gen_stdout('Fail')
            self.inst.interpret_result()
            assert self.inst.result.result is status.FAIL

        def test_pass(self):
            """test.deqp.DEQPBaseTest.interpret_result: when Pass in result the
            result is 'Pass'.
            """
            self.inst.result.out = self.__gen_stdout('Pass')
            self.inst.interpret_result()
            assert self.inst.result.result is status.PASS

        def test_warn(self):
            """test.deqp.DEQPBaseTest.interpret_result: when QualityWarning in
            result the result is 'warn'.
            """
            self.inst.result.out = self.__gen_stdout('QualityWarning')
            self.inst.interpret_result()
            assert self.inst.result.result is status.WARN

        def test_error(self):
            """test.deqp.DEQPBaseTest.interpret_result: when InternalError in
            result the result is 'fail'.
            """
            self.inst.result.out = self.__gen_stdout('InternalError')
            self.inst.interpret_result()
            assert self.inst.result.result is status.FAIL

        def test_crash(self):
            """test.deqp.DEQPBaseTest.interpret_result: when InternalError in
            result the result is 'crash'.
            """
            self.inst.result.out = self.__gen_stdout('Crash')
            self.inst.interpret_result()
            assert self.inst.result.result is status.CRASH

        def test_skip(self):
            """test.deqp.DEQPBaseTest.interpret_result: when NotSupported in
            result the result is 'skip'.
            """
            self.inst.result.out = self.__gen_stdout('NotSupported')
            self.inst.interpret_result()
            assert self.inst.result.result is status.SKIP

        def test_resourceerror(self):
            """test.deqp.DEQPBaseTest.interpret_result: when ResourceError in
            result the result is 'crash'.
            """
            self.inst.result.out = self.__gen_stdout('ResourceError')
            self.inst.interpret_result()
            assert self.inst.result.result is status.CRASH


class TestGenMustpassTests(object):
    """Tests for the gen_mustpass_tests function."""

    _txt = """\
dEQP.piglit.group1.test1
dEQP.piglit.group1.test2
dEQP.piglit.nested.group2.test3
dEQP.piglit.nested.group2.test4
    """

    def test_basic(self, tmpdir):
        p = tmpdir.join('foo.txt')
        p.write(self._txt)
        tests = set(deqp.gen_mustpass_tests(six.text_type(p)))
        print(tests)
        assert tests == {
            'dEQP.piglit.group1.test1',
            'dEQP.piglit.group1.test2',
            'dEQP.piglit.nested.group2.test3',
            'dEQP.piglit.nested.group2.test4',
        }
