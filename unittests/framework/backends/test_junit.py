# coding=utf-8
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

"""Tests for the Junit backend package."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import textwrap
try:
    from lxml import etree
except ImportError:
    import xml.etree.cElementTree as etree
try:
    import mock
except ImportError:
    from unittest import mock

import pytest
import six

from framework import backends
from framework import grouptools
from framework import results
from framework import status

from . import shared

# pylint: disable=no-self-use

JUNIT_SCHEMA = os.path.join(os.path.dirname(__file__), 'schema', 'junit-7.xsd')

_XML = """\
<?xml version='1.0' encoding='utf-8'?>
  <testsuites>
    <testsuite name="piglit" tests="1">
      <testcase classname="piglit.foo.bar" name="a-test" status="pass" time="1.12345">
        <system-out>this/is/a/command\nThis is stdout</system-out>
        <system-err>this is stderr

pid: [1934]
time start: 1.0
time end: 4.5
        </system-err>
      </testcase>
    </testsuite>
  </testsuites>
"""

@pytest.yield_fixture(autouse=True, scope="module")
def mock_compression():
    with mock.patch('framework.backends.compression.get_mode',
                    mock.Mock(return_value='none')):
        yield


class TestProtectedLoad(object):
    """Tests for the _load method."""

    def test_default_name(self, tmpdir):
        """backends.junit._load: uses 'junit result' for name as fallback."""
        tmpdir.chdir()
        with open('results.xml', 'w') as f:
            f.write(_XML)
        test = backends.junit.REGISTRY.load('results.xml', 'none')

        assert test.name == 'junit result'

    def test_file_name(self, tmpdir):
        """backends.junit._load: uses the filename for name if filename !=
        'results'
        """
        p = tmpdir.join('foobar.xml')
        p.write(_XML)
        test = backends.junit.REGISTRY.load(six.text_type(p), 'none')
        assert test.name == 'foobar'

    def test_folder_name(self, tmpdir):
        """backends.junit._load: uses the folder name if the result is
        'results.'
        """
        tmpdir.mkdir('foo')
        p = tmpdir.join('foo', 'results.xml')
        p.write(_XML)
        test = backends.junit.REGISTRY.load(six.text_type(p), 'none')

        assert test.name == 'foo'

    class TestReturned(object):
        """Test that the returned object is as expected."""

        testname = grouptools.join('foo', 'bar', 'a-test')

        @pytest.fixture
        def result(self, tmpdir):
            p = tmpdir.join('test.xml')
            p.write(_XML)
            return backends.junit._load(six.text_type(p))

        def test_testrunresult(self, result):
            """backends.junit._load: returns a TestrunResult instance."""
            assert isinstance(result, results.TestrunResult)

        def test_replace_sep(self, result):
            """backends.junit._load: replaces '.' with grouptools.SEPARATOR."""
            assert self.testname in result.tests

        def test_testresult_instance(self, result):
            """backends.junit._load: replaces result with TestResult instance.
            """
            assert isinstance(result.tests[self.testname], results.TestResult)

        def test_status_instance(self, result):
            """backends.junit._load: a status is found and loaded."""
            assert isinstance(result.tests[self.testname].result,
                              status.Status)

        def test_time(self, result):
            time = result.tests[self.testname].time
            assert isinstance(time, results.TimeAttribute)
            assert time.start == 1.0
            assert time.end == 4.5

        def test_command(self, result):
            """backends.junit._load: command is loaded correctly."""
            assert result.tests[self.testname].command == 'this/is/a/command'

        def test_out(self, result):
            """backends.junit._load: stdout is loaded correctly."""
            assert result.tests[self.testname].out == 'This is stdout'

        def test_err(self, result):
            """backends.junit._load: stderr is loaded correctly."""
            expected = textwrap.dedent("""\
                this is stderr

                pid: [1934]
                time start: 1.0
                time end: 4.5""")
            assert result.tests[self.testname].err.strip() == expected

        def test_totals(self, result):
            """backends.junit._load: Totals are calculated."""
            assert bool(result)

        def test_pid(self, result):
            """backends.junit._load: pid is loaded correctly."""
            assert result.tests[self.testname].pid == [1934]


class TestJUnitBackend(object):
    """Tests for the JUnitBackend class."""

    class TestFinalize(object):
        """Tests for the finalize method."""

        def test_skips_illformed_tests(self, tmpdir):
            """backends.junit.JUnitBackend: skips illformed tests"""
            result = results.TestResult()
            result.time.end = 1.2345
            result.result = 'pass'
            result.out = 'this is stdout'
            result.err = 'this is stderr'
            result.command = 'foo'

            test = backends.junit.JUnitBackend(six.text_type(tmpdir))
            test.initialize(shared.INITIAL_METADATA)
            with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
                t(result)
            tmpdir.join('tests', '1.xml').write('bad data')

            test.finalize()


class TestJUnitWriter(object):
    """Tests for the JUnitWriter class."""

    def test_junit_replace(self, tmpdir):
        """backends.junit.JUnitBackend.write_test: grouptools.SEPARATOR is
        replaced with '.'.
        """
        result = results.TestResult()
        result.time.end = 1.2345
        result.result = 'pass'
        result.out = 'this is stdout'
        result.err = 'this is stderr'
        result.command = 'foo'

        test = backends.junit.JUnitBackend(six.text_type(tmpdir))
        test.initialize(shared.INITIAL_METADATA)
        with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
            t(result)
        test.finalize()

        test_value = etree.parse(six.text_type(tmpdir.join('results.xml')))
        test_value = test_value.getroot()

        assert test_value.find('.//testcase').attrib['classname'] == \
            'piglit.a.group'

    class TestValid(object):
        @pytest.fixture
        def test_file(self, tmpdir):
            tmpdir.mkdir('foo')
            p = tmpdir.join('foo')

            result = results.TestResult()
            result.time.end = 1.2345
            result.result = 'pass'
            result.out = 'this is stdout'
            result.err = 'this is stderr'
            result.command = 'foo'
            result.pid = 1034

            test = backends.junit.JUnitBackend(six.text_type(p))
            test.initialize(shared.INITIAL_METADATA)
            with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
                t(result)

            result.result = 'fail'
            with test.write_test(grouptools.join('a', 'test', 'test1')) as t:
                t(result)
            test.finalize()

            return six.text_type(p.join('results.xml'))

        def test_xml_well_formed(self, test_file):
            """backends.junit.JUnitBackend.write_test: produces well formed xml."""
            etree.parse(test_file)

        @pytest.mark.skipif(etree.__name__ != 'lxml.etree',
                            reason="This test requires lxml")
        def test_xml_valid(self, test_file):
            """backends.junit.JUnitBackend.write_test: produces valid JUnit xml."""
            # This XMLSchema class is unique to lxml
            schema = etree.XMLSchema(file=JUNIT_SCHEMA)  # pylint: disable=no-member
            with open(test_file, 'r') as f:
                assert schema.validate(etree.parse(f))


class TestJUnitSubtestWriter(object):
    """Tests for the JUnitWriter class."""

    def test_junit_replace(self, tmpdir):
        """backends.junit.JUnitBackend.write_test: grouptools.SEPARATOR is
        replaced with '.'.
        """
        result = results.TestResult()
        result.time.end = 1.2345
        result.out = 'this is stdout'
        result.err = 'this is stderr'
        result.command = 'foo'
        result.subtests['foo'] = 'pass'
        result.subtests['bar'] = 'fail'

        test = backends.junit.JUnitBackend(six.text_type(tmpdir),
                                           junit_subtests=True)
        test.initialize(shared.INITIAL_METADATA)
        with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
            t(result)
        test.finalize()

        test_value = etree.parse(six.text_type(tmpdir.join('results.xml')))
        test_value = test_value.getroot()

        assert test_value.find('.//testsuite/testsuite').attrib['name'] == \
            'piglit.a.group.test1'

    def test_junit_replace_suffix(self, tmpdir):
        """backends.junit.JUnitBackend.write_test: grouptools.SEPARATOR is
        replaced with '.'.
        """
        result = results.TestResult()
        result.time.end = 1.2345
        result.out = 'this is stdout'
        result.err = 'this is stderr'
        result.command = 'foo'
        result.subtests['foo'] = 'pass'
        result.subtests['bar'] = 'fail'

        test = backends.junit.JUnitBackend(six.text_type(tmpdir),
                                           junit_subtests=True,
                                           junit_suffix='.foo')
        test.initialize(shared.INITIAL_METADATA)
        with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
            t(result)
        test.finalize()

        test_value = etree.parse(six.text_type(tmpdir.join('results.xml')))
        test_value = test_value.getroot()

        suite = test_value.find('.//testsuite/testsuite')
        assert suite.attrib['name'] == 'piglit.a.group.test1'
        assert suite.find('.//testcase[@name="{}"]'.format('foo.foo')) is not None

    def test_subtest_skip(self, tmpdir):
        result = results.TestResult()
        result.time.end = 1.2345
        result.out = 'this is stdout'
        result.err = 'this is stderr'
        result.command = 'foo'
        result.subtests['foo'] = 'pass'
        result.subtests['bar'] = 'skip'

        test = backends.junit.JUnitBackend(six.text_type(tmpdir),
                                           junit_subtests=True)
        test.initialize(shared.INITIAL_METADATA)
        with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
            t(result)
        test.finalize()

        test_value = etree.parse(six.text_type(tmpdir.join('results.xml')))
        test_value = test_value.getroot()

        suite = test_value.find('.//testsuite/testsuite')
        assert suite.attrib['name'] == 'piglit.a.group.test1'
        assert suite.find('.//testcase[@name="{}"]/skipped'.format('bar')) \
            is not None

    def test_result_skip(self, tmpdir):
        result = results.TestResult()
        result.time.end = 1.2345
        result.out = 'this is stdout'
        result.err = 'this is stderr'
        result.command = 'foo'
        result.result = 'skip'

        test = backends.junit.JUnitBackend(six.text_type(tmpdir),
                                           junit_subtests=True)
        test.initialize(shared.INITIAL_METADATA)
        with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
            t(result)
        test.finalize()

        test_value = etree.parse(six.text_type(tmpdir.join('results.xml')))
        test_value = test_value.getroot()

        elem = test_value.find('.//testsuite/testcase[@name="test1"]/skipped')
        assert elem is not None

    def test_classname(self, tmpdir):
        result = results.TestResult()
        result.time.end = 1.2345
        result.out = 'this is stdout'
        result.err = 'this is stderr'
        result.command = 'foo'
        result.subtests['foo'] = 'pass'
        result.subtests['bar'] = 'skip'

        test = backends.junit.JUnitBackend(six.text_type(tmpdir),
                                           junit_subtests=True)
        test.initialize(shared.INITIAL_METADATA)
        with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
            t(result)
        test.finalize()

        test_value = etree.parse(six.text_type(tmpdir.join('results.xml')))
        test_value = test_value.getroot()

        suite = test_value.find('.//testsuite/testsuite')
        assert suite.find('.//testcase[@classname="piglit.a.group.test1"]') \
            is not None

    class TestValid(object):
        @pytest.fixture
        def test_file(self, tmpdir):
            tmpdir.mkdir('foo')
            p = tmpdir.join('foo')

            result = results.TestResult()
            result.time.end = 1.2345
            result.out = 'this is stdout'
            result.err = 'this is stderr'
            result.command = 'foo'
            result.pid = 1034
            result.subtests['foo'] = 'pass'
            result.subtests['bar'] = 'fail'

            test = backends.junit.JUnitBackend(six.text_type(p),
                                               junit_subtests=True)
            test.initialize(shared.INITIAL_METADATA)
            with test.write_test(grouptools.join('a', 'group', 'test1')) as t:
                t(result)

            result.result = 'fail'
            with test.write_test(grouptools.join('a', 'test', 'test1')) as t:
                t(result)
            test.finalize()

            return six.text_type(p.join('results.xml'))

        def test_xml_well_formed(self, test_file):
            """backends.junit.JUnitBackend.write_test: produces well formed xml."""
            etree.parse(test_file)

        @pytest.mark.skipif(etree.__name__ != 'lxml.etree',
                            reason="This test requires lxml")
        def test_xml_valid(self, test_file):
            """backends.junit.JUnitBackend.write_test: produces valid JUnit xml."""
            # This XMLSchema class is unique to lxml
            schema = etree.XMLSchema(file=JUNIT_SCHEMA)  # pylint: disable=no-member
            with open(test_file, 'r') as f:
                assert schema.validate(etree.parse(f))
