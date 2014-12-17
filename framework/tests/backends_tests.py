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

import os
try:
    from lxml import etree
except ImportError:
    import xml.etree.cElementTree as etree
try:
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt
import framework.core as core
import framework.results as results
import framework.backends as backends
import framework.tests.utils as utils


BACKEND_INITIAL_META = {
    'name': 'name',
    'test_count': 0,
    'env': {},
    'options': {k: v for k, v in core.Options()},
}

JUNIT_SCHEMA = 'framework/tests/schema/junit-7.xsd'


def test_initialize_jsonbackend():
    """ Test that JSONBackend initializes

    This needs to be handled separately from the others because it requires
    arguments

    """
    with utils.tempdir() as tdir:
        func = results.JSONBackend(tdir)
        assert isinstance(func, results.JSONBackend)


@utils.nose_generator
def test_get_backend():
    """ Generate tests to get various backends """
    # We use a hand generated list here to ensure that we are getting what we
    # expect
    backends_ = {
        'json': backends.JSONBackend,
        'junit': backends.JUnitBackend,
    }

    check = lambda n, i: nt.assert_is(backends.get_backend(n), i)

    for name, inst in backends_.iteritems():
        check.description = 'get_backend({0}) returns {0} backend'.format(name)
        yield check, name, inst


class TestJunitNoTests(utils.StaticDirectory):
    @classmethod
    def setup_class(cls):
        super(TestJunitNoTests, cls).setup_class()
        test = backends.JUnitBackend(cls.tdir)
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
        test = backends.JUnitBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(
            'a/test/group/test1',
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
        schema = etree.XMLSchema(file=JUNIT_SCHEMA)
        with open(self.test_file, 'r') as f:
            assert schema.validate(etree.parse(f)), 'xml is not valid'


class TestJUnitMultiTest(TestJUnitSingleTest):
    @classmethod
    def setup_class(cls):
        super(TestJUnitMultiTest, cls).setup_class()
        cls.test_file = os.path.join(cls.tdir, 'results.xml')
        test = backends.JUnitBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(
            'a/test/group/test1',
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
        """ JUnitBackend.write_test() (twice) produces well formed xml """
        super(TestJUnitMultiTest, self).test_xml_well_formed()

    def test_xml_valid(self):
        """ JUnitBackend.write_test() (twice) produces valid xml """
        super(TestJUnitMultiTest, self).test_xml_valid()


def test_json_initialize_metadata():
    """ JSONBackend.initialize() produces a metadata.json file """
    with utils.tempdir() as f:
        test = backends.JSONBackend(f)
        test.initialize(BACKEND_INITIAL_META)

        nt.ok_(os.path.exists(os.path.join(f, 'metadata.json')))


class TestJSONTestMethod(utils.StaticDirectory):
    @classmethod
    def setup_class(cls):
        cls.test_name = 'a/test/group/test1'
        cls.result = results.TestResult({
            'time': 1.2345,
            'result': 'pass',
            'out': 'this is stdout',
            'err': 'this is stderr',
        })
        super(TestJSONTestMethod, cls).setup_class()
        test = backends.JSONBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(cls.test_name, cls.result)

    def test_write_test(self):
        """ JSONBackend.write_test() adds tests to a 'tests' directory """
        assert os.path.exists(os.path.join(self.tdir, 'tests', '0.json'))

    def test_json_is_valid(self):
        """ JSONBackend.write_test() produces valid json """
        with open(os.path.join(self.tdir, 'tests', '0.json'), 'r') as f:
            try:
                json.load(f)
            except Exception as e:
                raise AssertionError(e)

    def test_json_is_correct(self):
        """ JSONBackend.write_test() produces correct json """
        with open(os.path.join(self.tdir, 'tests', '0.json'), 'r') as f:
            test = json.load(f)

        nt.assert_dict_equal({self.test_name: self.result}, test)


class TestJSONTestFinalize(utils.StaticDirectory):
    @classmethod
    def setup_class(cls):
        cls.test_name = 'a/test/group/test1'
        cls.result = results.TestResult({
            'time': 1.2345,
            'result': 'pass',
            'out': 'this is stdout',
            'err': 'this is stderr',
        })
        super(TestJSONTestFinalize, cls).setup_class()
        test = backends.JSONBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(cls.test_name, cls.result)
        test.finalize()

    def test_remove_metadata(self):
        """ JSONBackend.finalize() removes metadata.json """
        assert not os.path.exists(os.path.join(self.tdir, 'metadata.json'))

    def test_remove_tests(self):
        """ JSONBackend.finalize() removes tests directory """
        assert not os.path.exists(os.path.join(self.tdir, 'tests'))

    def test_create_results(self):
        """ JSONBackend.finalize() creates a results.json file """
        assert os.path.exists(os.path.join(self.tdir, 'results.json'))

    def test_results_valid(self):
        """ JSONBackend.finalize() results.json is valid """
        with open(os.path.join(self.tdir, 'results.json'), 'r') as f:
            try:
                json.load(f)
            except Exception as e:
                raise AssertionError(e)


def test_junit_skips_bad_tests():
    """ backends.JUnitBackend skips illformed tests """
    with utils.tempdir() as tdir:
        test = backends.JUnitBackend(tdir)
        test.initialize(BACKEND_INITIAL_META)
        test.write_test(
            'a/test/group/test1',
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
