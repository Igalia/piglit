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

from framework import core, results, backends, grouptools
import framework.tests.utils as utils


BACKEND_INITIAL_META = {
    'name': 'name',
    'test_count': 0,
    'env': {},
    'options': {k: v for k, v in core.Options()},
}

JUNIT_SCHEMA = 'framework/tests/schema/junit-7.xsd'

doc_formatter = utils.DocFormatter({'seperator': grouptools.SEPARATOR})


@utils.nose_generator
def test_get_backend():
    """ Generate tests to get various backends """
    # We use a hand generated list here to ensure that we are getting what we
    # expect
    backends_ = {
        'json': backends.json.JSONBackend,
        'junit': backends.junit.JUnitBackend,
    }

    def check(n, i):
        return nt.assert_is(backends.get_backend(n), i)

    for name, inst in backends_.iteritems():
        check.description = 'get_backend({0}) returns {0} backend'.format(name)
        yield check, name, inst


class TestJunitNoTests(utils.StaticDirectory):
    @classmethod
    def setup_class(cls):
        super(TestJunitNoTests, cls).setup_class()
        test = backends.junit.JUnitBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.finalize()
        cls.test_file = os.path.join(cls.tdir, 'results.xml')

    def test_xml_well_formed(self):
        """ JUnitBackend.__init__ and finalize produce well formed xml

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
        """ JUnitBackend.write_test() (once) produces well formed xml """
        super(TestJUnitSingleTest, self).test_xml_well_formed()

    def test_xml_valid(self):
        """ JUnitBackend.write_test() (once) produces valid xml """
        if etree.__name__ != 'lxml.etree':
            raise SkipTest("Test requires lxml.")
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
            grouptools.join('a', 'different', 'test', 'group', 'test2'),
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
        """ JUnitBackend.write_test() (twice) produces well formed xml """
        super(TestJUnitMultiTest, self).test_xml_well_formed()

    def test_xml_valid(self):
        """ JUnitBackend.write_test() (twice) produces valid xml """
        super(TestJUnitMultiTest, self).test_xml_valid()


@doc_formatter
def test_junit_replace():
    """JUnitBackend.write_test: '{seperator}' is replaced with '.'"""
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
    """ backends.JUnitBackend skips illformed tests """
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
