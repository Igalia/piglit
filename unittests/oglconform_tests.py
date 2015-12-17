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

"""Tests for the oglconform integration."""

from StringIO import StringIO

import mock
import nose.tools as nt

from . import utils
from framework import grouptools

with mock.patch('framework.core.PIGLIT_CONFIG.required_get',
                mock.Mock(return_value='piglit.conf.example')):
    with mock.patch('subprocess.call', mock.Mock()):
        from tests import oglconform

# pylint: disable=protected-access,invalid-name,line-too-long


@mock.patch.object(oglconform.tempfile, 'NamedTemporaryFile')
def test_make_profile(mock_temp):
    """tests.oglconform._make_profile: Adds test names"""
    io_ = StringIO('group test.name\n')
    io_.name = mock.Mock()
    mock_file = mock.MagicMock()
    mock_file.__enter__.return_value = io_
    mock_temp.return_value = mock_file

    with mock.patch('subprocess.call', mock.Mock()):
        profile = oglconform._make_profile()

    name = grouptools.join('oglconform', 'group', 'test.name')
    nt.ok_(name in profile.test_list,
           msg='{} not in {}'.format(name, profile.test_list.keys()))


@utils.not_raises(ValueError)
@mock.patch.object(oglconform.tempfile, 'NamedTemporaryFile')
def test_make_profile_missing(mock_temp):
    """tests.oglconform._make_profile: handles missing groups"""
    io_ = StringIO('test.name\n')
    io_.name = mock.Mock()
    mock_file = mock.MagicMock()
    mock_file.__enter__.return_value = io_
    mock_temp.return_value = mock_file

    with mock.patch('subprocess.call', mock.Mock()):
        oglconform._make_profile()


def test_oglctest_command():
    """tests.oglconform.OGLCtest.command: value is as expected"""
    expected = ['piglit.conf.example', '-minFmt', '-v', '4', '-test', 'group',
                'test']

    test = oglconform.OGLCTest('group', 'test')
    nt.eq_(expected, test.command)


def test_oglctest_interpret_result_pass():
    """tests.oglconform.OGLCtest.interpret_result: status is pass when expected
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

    nt.eq_(test.result.result, 'pass')


def test_oglctest_interpret_result_skip():
    """tests.oglconform.OGLCtest.interpret_result: status is skip when tests not run"""
    test = oglconform.OGLCTest('group', 'test')
    test.result.returncode = 0
    test.result.out = (
        'Another line\n'
        'Total Failed : 0\n'
        'Total Passed : 0\n'
        'Total Not run: 1\n'
    )
    test.interpret_result()

    nt.eq_(test.result.result, 'skip')


@utils.nose_generator
def test_oglctest_interpret_result_skip_re():
    """Generate tests for various skip tests."""
    values = [
        'no test in schedule is compat',
        'GLSL 1.30 is not supported',
        'GLSL 1.40 is not supported',
        'GLSL 1.50 is not supported',
        'GLSL 3.30 is not supported',
        'GLSL 3.40 is not supported',
        'GLSL 3.50 is not supported',
        'wont be scheduled due to lack of compatible fbconfig'
    ]

    def test(out):
        """The actual test."""
        test = oglconform.OGLCTest('group', 'value')
        test.result.out = out
        test.result.returncode = 0
        test.interpret_result()
        nt.eq_(test.result.result, 'skip')

    for each in values:
        test.description = ('tests.oglconform.OGLCTest.interpret_result: '
                            '"{}" in result.out makes status skip'.format(each))
        yield test, each


def test_oglctest_interpret_result_fail():
    """tests.oglconform.OGLCtest.interpret_result: status is fail otherwise"""
    test = oglconform.OGLCTest('group', 'test')
    test.result.returncode = 0
    test.interpret_result()

    nt.eq_(test.result.result, 'fail')


def test_oglctest_interpret_result_crash():
    """tests.oglconform.OGLCtest.interpret_result: status is crash if returncode is not 0"""
    test = oglconform.OGLCTest('group', 'test')
    test.interpret_result()

    nt.eq_(test.result.result, 'crash')
