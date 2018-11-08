# coding=utf-8
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

"""Tests for the piglit_test module."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import textwrap
try:
    from unittest import mock
except ImportError:
    import mock

import pytest


from framework import status
from framework.options import _Options as Options
from framework.test.base import TestIsSkip as _TestIsSkip
from framework.test.piglit_test import PiglitBaseTest, PiglitGLTest

# pylint: disable=no-self-use
# pylint: disable=protected-access


class TestPiglitBaseTest(object):
    """Tests for the PiglitBaseTest class."""

    class TestIntepretResult(object):
        """Tests for PiglitBaseTest.interpret_results."""

        def test_basic(self):
            """A basic sanity test with nothing tricky."""
            test = PiglitBaseTest(['foo'])
            test.result.out = 'PIGLIT: {"result": "pass"}\n'
            test.result.returncode = 0
            test.interpret_result()
            assert test.result.result is status.PASS

        def test_stdout(self):
            """Separates the actual stdout printing from the PIGLIT protocol.
            """
            test = PiglitBaseTest(['foo'])
            test.result.out = textwrap.dedent("""\
                This is output

                more output
                PIGLIT: {"result": "pass"}
                and stuff""")
            test.result.returncode = 0
            test.interpret_result()

            assert test.result.result is status.PASS
            assert test.result.out == textwrap.dedent("""\
                This is output

                more output
                and stuff""")

        def test_with_subtests(self):
            """works with one subtest."""
            test = PiglitBaseTest(['foo'])
            test.result.out = textwrap.dedent("""\
                PIGLIT: {"result": "pass"}
                PIGLIT: {"subtest": {"subtest": "pass"}}""")
            test.result.returncode = 0
            test.interpret_result()
            assert test.result.subtests['subtest'] is status.PASS

        def test_with_multiple_subtests(self):
            """Works with multiple subtests.

            Including that it doesn't only take the last subtest, but counts
            all of them.
            """
            test = PiglitBaseTest(['a', 'command'])
            test.result.out = textwrap.dedent("""\
                PIGLIT: {"result": "pass"}
                PIGLIT: {"subtest": {"test1": "pass"}}
                PIGLIT: {"subtest": {"test2": "pass"}}""")
            test.result.returncode = 0
            test.interpret_result()

            assert dict(test.result.subtests) == \
                {'test1': 'pass', 'test2': 'pass'}


class TestPiglitGLTest(object):
    """tests for the PiglitGLTest class."""

    class TestCommand(object):
        """Tests for the command getter and setter."""

        def test_getter_serial(self):
            """adds -auto to serial tests."""
            test = PiglitGLTest(['foo'])
            assert '-auto' in test.command

        def test_getter_concurrent(self):
            """adds -fbo and -auto to concurrent tests."""
            test = PiglitGLTest(['foo'], run_concurrent=True)
            assert '-auto' in test.command
            assert '-fbo' in test.command

        def test_setter_no_add_auto(self):
            """Doesn't add -fbo or -auto when setting."""
            test = PiglitGLTest(['foo'], run_concurrent=True)
            test.command += ['bar']
            assert '-auto' not in test._command
            assert '-fbo' not in test._command

    class TestIsSkip(object):
        """Tests for the is_skip method and the constructor logic to make it
        work.
        """

        @pytest.yield_fixture()
        def mock_options(self):
            with mock.patch('framework.test.piglit_test.options.OPTIONS',
                            new_callable=Options) as m:
                yield m

        def test_include_and_exclude(self):
            """ raises if include and exclude are given."""
            with pytest.raises(AssertionError):
                PiglitGLTest(['foo'],
                             require_platforms=['glx'],
                             exclude_platforms=['gbm'])

        def test_platform_in_require(self, mock_options):
            """does not skip if platform is in require_platforms."""
            mock_options.env['PIGLIT_PLATFORM'] = 'glx'
            test = PiglitGLTest(['foo'], require_platforms=['glx'])
            test.is_skip()

        def test_platform_not_in_require(self, mock_options):
            """skips if platform is not in require_platforms."""
            mock_options.env['PIGLIT_PLATFORM'] = 'gbm'
            test = PiglitGLTest(['foo'], require_platforms=['glx'])
            with pytest.raises(_TestIsSkip):
                test.is_skip()

        def test_platform_in_exclude(self, mock_options):
            """skips if platform is in exclude_platforms."""
            mock_options.env['PIGLIT_PLATFORM'] = 'glx'
            test = PiglitGLTest(['foo'], exclude_platforms=['glx'])
            with pytest.raises(_TestIsSkip):
                test.is_skip()

        def test_platform_not_in_exclude(self, mock_options):
            """does not skip if platform is in exclude_platforms."""
            mock_options.env['PIGLIT_PLATFORM'] = 'gbm'
            test = PiglitGLTest(['foo'], exclude_platforms=['glx'])
            test.is_skip()
