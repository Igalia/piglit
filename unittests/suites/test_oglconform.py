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

"""Tests for the oglconform integration."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
try:
    import mock
except ImportError:
    from unittest import mock

import pytest
from six.moves import cStringIO as StringIO

from framework import grouptools
from framework import status

with mock.patch('framework.core.PIGLIT_CONFIG.required_get',
                mock.Mock(return_value='piglit.conf.example')):
    with mock.patch('subprocess.call', mock.Mock()):
        from tests import oglconform

# pylint: disable=protected-access,no-self-use


class TestMakeProfile(object):
    """Tests for the _make_profile function."""

    def test_basic(self, mocker):
        """tests.oglconform._make_profile: Adds test names"""
        io_ = mocker.Mock(wraps=StringIO(u'group test.name\n'))
        io_.name = mocker.Mock()

        mock_file = mocker.MagicMock()
        mock_file.__enter__.return_value = io_

        mock_temp = mocker.patch.object(oglconform.tempfile,
                                        'NamedTemporaryFile')
        mock_temp.return_value = mock_file

        mocker.patch('subprocess.call', mocker.Mock())
        profile = oglconform._make_profile()

        name = grouptools.join('oglconform', 'group', 'test.name')
        assert name in profile.test_list

    def test_missing(self, mocker):
        """tests.oglconform._make_profile: handles missing groups"""
        io_ = mocker.Mock(wraps=StringIO(u'test.name\n'))
        io_.name = mocker.Mock()

        mock_file = mocker.MagicMock()
        mock_file.__enter__.return_value = io_

        mock_temp = mocker.patch.object(oglconform.tempfile,
                                        'NamedTemporaryFile')
        mock_temp.return_value = mock_file

        mocker.patch('subprocess.call', mocker.Mock())
        oglconform._make_profile()


class TestOGLCTest(object):
    """Tests for the OGLCTest class."""

    def test_command(self):
        """tests.oglconform.OGLCtest.command: value is as expected"""
        expected = ['piglit.conf.example', '-minFmt', '-v', '4', '-test',
                    'group', 'test']

        test = oglconform.OGLCTest('group', 'test')
        assert expected == test.command

    class TestInterpretResult(object):
        """Tests for the interpret_result method."""

        def test_pass(self):
            """tests.oglconform.OGLCtest.interpret_result: status is pass when
            expected.
            """
            test = oglconform.OGLCTest('group', 'test')
            test.result.returncode = 0
            test.result.out = (
                'Another line\n'
                'Total Passed : 1\n'
                'Total Failed : 0\n'
                'Total Not run: 0\n'
            )
            test.interpret_result()

            assert test.result.result is status.PASS

        def test_skip_from_not_run(self):
            """tests.oglconform.OGLCtest.interpret_result: status is skip when
            tests not run.
            """
            test = oglconform.OGLCTest('group', 'test')
            test.result.returncode = 0
            test.result.out = (
                'Another line\n'
                'Total Failed : 0\n'
                'Total Passed : 0\n'
                'Total Not run: 1\n'
            )
            test.interpret_result()

            assert test.result.result is status.SKIP

        def test_fail(self):
            """tests.oglconform.OGLCtest.interpret_result: status is fail
            otherwise.
            """
            test = oglconform.OGLCTest('group', 'test')
            test.result.returncode = 0
            test.interpret_result()

            assert test.result.result is status.FAIL

        def test_crash(self):
            """tests.oglconform.OGLCtest.interpret_result: status is crash if
            returncode is not 0.
            """
            test = oglconform.OGLCTest('group', 'test')
            test.result.returncode = -1
            test.interpret_result()

            assert test.result.result is status.CRASH

    @pytest.mark.parametrize("out", [
        'no test in schedule is compat',
        'GLSL 1.30 is not supported',
        'GLSL 1.40 is not supported',
        'GLSL 1.50 is not supported',
        'GLSL 3.30 is not supported',
        'GLSL 3.40 is not supported',
        'GLSL 3.50 is not supported',
        'wont be scheduled due to lack of compatible fbconfig',
    ])
    def test_skip_from_re(self, out):
        test = oglconform.OGLCTest('group', 'value')
        test.result.out = out
        test.result.returncode = 0
        test.interpret_result()
        assert test.result.result == 'skip'
