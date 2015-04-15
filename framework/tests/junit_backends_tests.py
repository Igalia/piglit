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

# pylint: disable=missing-docstring

""" Tests for the backend package """

from __future__ import print_function, absolute_import
import os

try:
    from lxml import etree
except ImportError:
    import xml.etree.cElementTree as etree
import nose.tools as nt
from nose.plugins.skip import SkipTest

from framework import results, backends, grouptools, status
import framework.tests.utils as utils
from .backends_tests import BACKEND_INITIAL_META


JUNIT_SCHEMA = 'framework/tests/schema/junit-7.xsd'

doc_formatter = utils.DocFormatter({'separator': grouptools.SEPARATOR})

_XML =  """\
<?xml version='1.0' encoding='utf-8'?>
  <testsuites>
    <testsuite name="piglit" tests="1">
      <testcase classname="piglit.foo.bar" name="a-test" status="pass" time="1.12345">
        <system-out>this/is/a/command\nThis is stdout</system-out>
        <system-err>this is stderr</system-err>
      </testcase>
    </testsuite>
  </testsuites>
"""


class TestJunitNoTests(utils.StaticDirectory):
    @classmethod
    def setup_class(cls):
        super(TestJunitNoTests, cls).setup_class()
        test = backends.junit.JUnitBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.finalize()
        cls.test_file = os.path.join(cls.tdir, 'results.xml')

    def test_xml_well_formed(self):
        """backends.junit.JUnitBackend: initialize and finalize produce well formed xml

        While it will produce valid XML, it cannot produc valid JUnit, since
        JUnit requires at least one test to be valid

        """
        try:
            etree.parse(self.test_file)
        except Exception as e:
            raise AssertionError(e)


class TestJUnitSingleTest(TestJunitNoTests):
    @classmethod
    def setup_class(cls):
        super(TestJUnitSingleTest, cls).setup_class()
        cls.test_file = os.path.join(cls.tdir, 'results.xml')
        test = backends.junit.JUnitBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(
            grouptools.join('a', 'test', 'group', 'test1'),
            results.TestResult({
                'time': 1.2345,
                'result': 'pass',
                'out': 'this is stdout',
                'err': 'this is stderr',
                'command': 'foo',
            })
        )
        test.finalize()

    def test_xml_well_formed(self):
        """backends.junit.JUnitBackend.write_test(): (once) produces well formed xml"""
        super(TestJUnitSingleTest, self).test_xml_well_formed()

    def test_xml_valid(self):
        """backends.junit.JUnitBackend.write_test(): (once) produces valid xml"""
        if etree.__name__ != 'lxml.etree':
            raise SkipTest('Test requires lxml features')
        schema = etree.XMLSchema(file=JUNIT_SCHEMA)
        with open(self.test_file, 'r') as f:
            assert schema.validate(etree.parse(f)), 'xml is not valid'


class TestJUnitMultiTest(TestJUnitSingleTest):
    @classmethod
    def setup_class(cls):
        super(TestJUnitMultiTest, cls).setup_class()
        cls.test_file = os.path.join(cls.tdir, 'results.xml')
        test = backends.junit.JUnitBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(
            grouptools.join('a', 'test', 'group', 'test1'),
            results.TestResult({
                'time': 1.2345,
                'result': 'pass',
                'out': 'this is stdout',
                'err': 'this is stderr',
                'command': 'foo',
            })
        )
        test.write_test(
            'a/different/test/group/test2',
            results.TestResult({
                'time': 1.2345,
                'result': 'fail',
                'out': 'this is stdout',
                'err': 'this is stderr',
                'command': 'foo',
            })
        )
        test.finalize()

    def test_xml_well_formed(self):
        """backends.junit.JUnitBackend.write_test(): (twice) produces well formed xml"""
        super(TestJUnitMultiTest, self).test_xml_well_formed()

    def test_xml_valid(self):
        """backends.junit.JUnitBackend.write_test(): (twice) produces valid xml"""
        super(TestJUnitMultiTest, self).test_xml_valid()


@doc_formatter
def test_junit_replace():
    """backends.junit.JUnitBackend.write_test(): '{separator}' is replaced with '.'"""
    with utils.tempdir() as tdir:
        test = backends.junit.JUnitBackend(tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(
            grouptools.join('a', 'test', 'group', 'test1'),
            results.TestResult({
                'time': 1.2345,
                'result': 'pass',
                'out': 'this is stdout',
                'err': 'this is stderr',
                'command': 'foo',
            })
        )
        test.finalize()

        test_value = etree.parse(os.path.join(tdir, 'results.xml')).getroot()

    nt.assert_equal(test_value.find('.//testcase').attrib['classname'],
                    'piglit.a.test.group')


def test_junit_skips_bad_tests():
    """backends.junit.JUnitBackend: skips illformed tests"""
    with utils.tempdir() as tdir:
        test = backends.junit.JUnitBackend(tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(
            grouptools.join('a', 'test', 'group', 'test1'),
            results.TestResult({
                'time': 1.2345,
                'result': 'pass',
                'out': 'this is stdout',
                'err': 'this is stderr',
                'command': 'foo',
            })
        )
        with open(os.path.join(tdir, 'tests', '1.xml'), 'w') as f:
            f.write('bad data')

        try:
            test.finalize()
        except etree.ParseError as e:
            raise AssertionError(e)


class TestJUnitLoad(utils.StaticDirectory):
    """Methods that test loading JUnit results."""
    __instance = None

    @classmethod
    def setup_class(cls):
        super(TestJUnitLoad, cls).setup_class()
        cls.xml_file = os.path.join(cls.tdir, 'results.xml')
        
        with open(cls.xml_file, 'w') as f:
            f.write(_XML)

        cls.testname = grouptools.join('foo', 'bar', 'a-test')

    @classmethod
    def xml(cls):
        if cls.__instance is None:
            cls.__instance =  backends.junit._load(cls.xml_file)
        return cls.__instance

    @utils.no_error
    def test_no_errors(self):
        """backends.junit._load: Raises no errors for valid junit."""
        self.xml()

    def test_return_testrunresult(self):
        """backends.junit._load: returns a TestrunResult instance"""
        nt.assert_is_instance(self.xml(), results.TestrunResult)

    @doc_formatter
    def test_replace_sep(self):
        """backends.junit._load: replaces '.' with '{separator}'"""
        nt.assert_in(self.testname, self.xml().tests)

    def test_testresult_instance(self):
        """backends.junit._load: replaces result with TestResult instance."""
        nt.assert_is_instance(self.xml().tests[self.testname], results.TestResult)

    def test_status_instance(self):
        """backends.junit._load: a status is found and loaded."""
        nt.assert_is_instance(self.xml().tests[self.testname]['result'],
                              status.Status)

    def test_time(self):
        """backends.junit._load: Time is loaded correctly."""
        time = self.xml().tests[self.testname]['time']
        nt.assert_is_instance(time, float)
        nt.assert_equal(time, 1.12345)

    def test_command(self):
        """backends.junit._load: command is loaded correctly."""
        test = self.xml().tests[self.testname]['command']
        nt.assert_equal(test, 'this/is/a/command')

    def test_out(self):
        """backends.junit._load: stdout is loaded correctly."""
        test = self.xml().tests[self.testname]['out']
        nt.assert_equal(test, 'This is stdout')

    def test_err(self):
        """backends.junit._load: stderr is loaded correctly."""
        test = self.xml().tests[self.testname]['err']
        nt.assert_equal(test, 'this is stderr')

    @utils.no_error
    def test_load_file(self):
        """backends.junit.load: Loads a file directly."""
        backends.junit.REGISTRY.load(self.xml_file)

    @utils.no_error
    def test_load_dir(self):
        """backends.junit.load: Loads a directory."""
        backends.junit.REGISTRY.load(self.tdir)


def test_load_file_name():
    """backends.junit._load: uses the filename for name if filename != 'results'
    """
    with utils.tempdir() as tdir:
        filename = os.path.join(tdir, 'foobar.xml')
        with open(filename, 'w') as f:
            f.write(_XML)

        test = backends.junit.REGISTRY.load(filename)
    nt.assert_equal(test.name, 'foobar')


def test_load_folder_name():
    """backends.junit._load: uses the foldername if the result is 'results'
    """
    with utils.tempdir() as tdir:
        os.mkdir(os.path.join(tdir, 'a cool test'))
        filename = os.path.join(tdir, 'a cool test', 'results.xml')
        with open(filename, 'w') as f:
            f.write(_XML)

        test = backends.junit.REGISTRY.load(filename)
    nt.assert_equal(test.name, 'a cool test')


@utils.test_in_tempdir
def test_load_default_name():
    """backends.junit._load: uses 'junit result' for name as fallback"""
    with utils.tempdir() as tdir:
        os.chdir(tdir)

        filename = 'results.xml'
        with open(filename, 'w') as f:
            f.write(_XML)

        test = backends.junit.REGISTRY.load(filename)

    nt.assert_equal(test.name, 'junit result')
