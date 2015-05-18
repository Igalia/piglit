# Copyright (c) 2015 Intel Corporation

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

import nose.tools as nt

from framework import profile, grouptools, exceptions
from framework.core import PIGLIT_CONFIG
from framework.test import deqp
from framework.tests import utils

# pylint:disable=line-too-long,invalid-name

doc_formatter = utils.DocFormatter({'separator': grouptools.SEPARATOR})


class _DEQPTestTest(deqp.DEQPBaseTest):
    deqp_bin = 'deqp.bin'
    extra_args = ['extra']


@utils.set_piglit_conf(('deqp_test', 'test_env', 'from conf'))
@utils.set_env(_PIGLIT_TEST_ENV='from env')
def test_get_option_env():
    """deqp.get_option: if env is set it overrides piglit.conf"""
    nt.eq_(deqp.get_option('_PIGLIT_TEST_ENV', ('deqp_test', 'test_env')),
           'from env')


@utils.set_piglit_conf(('deqp_test', 'test_env', 'from conf'))
@utils.set_env(_PIGLIT_TEST_ENV=None)
def test_get_option_conf():
    """deqp.get_option: if env is not set a value is taken from piglit.conf"""
    nt.eq_(deqp.get_option('_PIGLIT_TEST_ENV', ('deqp_test', 'test_env')),
           'from conf')


@utils.set_env(_PIGLIT_TEST_ENV=None)
def test_get_option_conf_no_section():
    """deqp.get_option: if a no_section error is raised and env is unset None is return
    """
    assert not PIGLIT_CONFIG.has_section('deqp_test')
    nt.eq_(deqp.get_option('_PIGLIT_TEST_ENV', ('deqp_test', 'test_env')), None)


# The first argument ensures the sectio exists
@utils.set_piglit_conf(('deqp_test', 'test_env', 'from conf'),
                       ('deqp_test', 'not_exists', None))
@utils.set_env(_PIGLIT_TEST_ENV=None)
def test_get_option_conf_no_option():
    """deqp.get_option: if a no_option error is raised and env is unset None is return
    """
    nt.eq_(deqp.get_option('_PIGLIT_TEST_ENV', ('deqp_test', 'not_exists')),
           None)


class TestMakeProfile(object):
    """Test deqp.make_profile."""
    @classmethod
    def setup_class(cls):
        cls.profile = deqp.make_profile(['this.is.a.deqp.test'], _DEQPTestTest)

    def test_returns_profile(self):
        """deqp.make_profile: returns a TestProfile"""
        nt.assert_is_instance(self.profile, profile.TestProfile)

    @doc_formatter
    def test_grouptools(self):
        """deqp.make_profile: replaces '.' with '{separator}'"""
        nt.assert_in(grouptools.join('this', 'is', 'a', 'deqp', 'test'),
                     self.profile.test_list)


def test_iter_deqp_test_cases_test():
    """deqp.iter_deqp_test_cases: correctly detects a TEST: line"""
    with utils.tempfile('TEST: a.deqp.test') as tfile:
        gen = deqp.iter_deqp_test_cases(tfile)
        nt.eq_('a.deqp.test', next(gen))


def test_iter_deqp_test_cases_group():
    """deqp.iter_deqp_test_casesgen_caselist_txt: correctly detects a GROUP: line"""
    with utils.tempfile('GROUP: a group\nTEST: a.deqp.test') as tfile:
        gen = deqp.iter_deqp_test_cases(tfile)
        nt.eq_('a.deqp.test', next(gen))


@nt.raises(exceptions.PiglitFatalError)
def test_iter_deqp_test_cases_bad():
    """deqp.iter_deqp_test_casesgen_caselist_txt: PiglitFatalException is raised if line is not TEST: or GROUP:
    """
    with utils.tempfile('this will fail') as tfile:
        gen = deqp.iter_deqp_test_cases(tfile)
        nt.eq_('a.deqp.test', next(gen))


@utils.no_error
def test_DEQPBaseTest_initialize():
    """deqp.DEQPBaseTest: can be initialized (with abstract methods overwritten)
    """
    _DEQPTestTest('a.deqp.test')


def test_DEQPBaseTest_command():
    """deqp.DEQPBaseTest.command: cls.extra_args are added to self.command"""
    test = _DEQPTestTest('a.deqp.test')
    nt.eq_(test.command[-1], 'extra')


def test_DEQPBaseTest_interpret_result_returncode():
    """deqp.DEQPBaseTest.interpret_result: if returncode is not 0 result is fail
    """
    test = _DEQPTestTest('a.deqp.test')
    test.result['returncode'] = 1
    test.interpret_result()

    nt.eq_(test.result['result'], 'fail')


def test_DEQPBaseTest_interpret_result_fallthrough():
    """deqp.DEQPBaseTest.interpret_result: if no case is hit set to fail
    """
    test = _DEQPTestTest('a.deqp.test')
    test.result['returncode'] = 0
    test.result['out'] = ''
    test.interpret_result()

    nt.eq_(test.result['result'], 'fail')


@utils.nose_generator
def test_DEQPBaseTest_interpret_result_status():
    """generate tests for each status possiblility."""
    def test(status, expected):
        inst = _DEQPTestTest('a.deqp.test')
        inst.result['returncode'] = 0
        inst.result['out'] = status
        inst.interpret_result()
        nt.eq_(inst.result['result'], expected)

    desc = ('deqp.DEQPBaseTest.interpret_result: '
            'when "{}" in stdout status is set to "{}"')

    _map = deqp.DEQPBaseTest._DEQPBaseTest__RESULT_MAP.iteritems()  # pylint: disable=no-member,protected-access

    for status, expected in _map:
        test.description = desc.format(status, expected)
        yield test, status, expected
