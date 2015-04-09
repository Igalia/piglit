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
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt

from framework import results, backends
import framework.tests.utils as utils
from .backends_tests import BACKEND_INITIAL_META


def test_initialize_jsonbackend():
    """ Test that JSONBackend initializes

    This needs to be handled separately from the others because it requires
    arguments

    """
    with utils.tempdir() as tdir:
        func = backends.json.JSONBackend(tdir)
        assert isinstance(func, backends.json.JSONBackend)


@utils.nose_generator
def test_get_backend():
    """ Generate tests to get various backends """
    # We use a hand generated list here to ensure that we are getting what we
    # expect
    backends_ = {
        'json': backends.json.JSONBackend,
        'junit': backends.junit.JUnitBackend,
    }

    check = lambda n, i: nt.assert_is(backends.get_backend(n), i)

    for name, inst in backends_.iteritems():
        check.description = 'get_backend({0}) returns {0} backend'.format(name)
        yield check, name, inst


def test_json_initialize_metadata():
    """ JSONBackend.initialize() produces a metadata.json file """
    with utils.tempdir() as f:
        test = backends.json.JSONBackend(f)
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
        test = backends.json.JSONBackend(cls.tdir)
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
        test = backends.json.JSONBackend(cls.tdir)
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


def test_update_results_current():
    """ update_results() returns early when the results_version is current """
    data = utils.JSON_DATA.copy()
    data['results_version'] = backends.json.CURRENT_JSON_VERSION

    with utils.tempdir() as d:
        with open(os.path.join(d, 'main'), 'w') as f:
            json.dump(data, f)

        with open(os.path.join(d, 'main'), 'r') as f:
            base = backends.json._load(f)

        res = backends.json._update_results(base, f.name)

    nt.assert_dict_equal(res.__dict__, base.__dict__)


def test_update_results_old():
    """ update_results() updates results

    Because of the design of the our updates (namely that they silently
    incrementally update from x to y) it's impossible to konw exactly what
    we'll get at th end without having tests that have to be reworked each time
    updates are run. Since there already is (at least for v0 -> v1) a fairly
    comprehensive set of tests, this test only tests that update_results() has
    been set equal to the CURRENT_JSON_VERSION, (which is one of the effects of
    runing update_results() with the assumption that there is sufficient other
    testing of the update process.

    """
    data = utils.JSON_DATA.copy()
    data['results_version'] = 0

    with utils.tempdir() as d:
        with open(os.path.join(d, 'main'), 'w') as f:
            json.dump(data, f)

        with open(os.path.join(d, 'main'), 'r') as f:
            base = backends.json._load(f)

        res = backends.json._update_results(base, f.name)

    nt.assert_equal(res.results_version, backends.json.CURRENT_JSON_VERSION)


@utils.no_error
def test_json_resume_non_folder():
    """ TestrunResult.resume doesn't accept a file """
    with utils.with_tempfile('') as f:
        with nt.assert_raises(AssertionError):
            backends.json._resume(f)


@utils.no_error
def test_resume_load():
    """ TestrunResult.resume loads with good results """
    with utils.tempdir() as f:
        backend = backends.json.JSONBackend(f)
        backend.initialize(BACKEND_INITIAL_META)
        backend.write_test("group1/test1", {'result': 'fail'})
        backend.write_test("group1/test2", {'result': 'pass'})
        backend.write_test("group2/test3", {'result': 'fail'})

        backends.json._resume(f)


def test_resume_load_valid():
    """ TestrunResult.resume loads valid results """
    with utils.tempdir() as f:
        backend = backends.json.JSONBackend(f)
        backend.initialize(BACKEND_INITIAL_META)
        backend.write_test("group1/test1", {'result': 'fail'})
        backend.write_test("group1/test2", {'result': 'pass'})
        backend.write_test("group2/test3", {'result': 'fail'})

        test = backends.json._resume(f)

        nt.assert_set_equal(
            set(test.tests.keys()),
            set(['group1/test1', 'group1/test2', 'group2/test3']),
        )


def test_resume_load_invalid():
    """ TestrunResult.resume ignores invalid results """
    with utils.tempdir() as f:
        backend = backends.json.JSONBackend(f)
        backend.initialize(BACKEND_INITIAL_META)
        backend.write_test("group1/test1", {'result': 'fail'})
        backend.write_test("group1/test2", {'result': 'pass'})
        backend.write_test("group2/test3", {'result': 'fail'})
        with open(os.path.join(f, 'tests', 'x.json'), 'w') as w:
            w.write('foo')

        test = backends.json._resume(f)

        nt.assert_set_equal(
            set(test.tests.keys()),
            set(['group1/test1', 'group1/test2', 'group2/test3']),
        )


@utils.no_error
def test_load_results_folder_as_main():
    """ Test that load_results takes a folder with a file named main in it """
    with utils.tempdir() as tdir:
        with open(os.path.join(tdir, 'main'), 'w') as tfile:
            tfile.write(json.dumps(utils.JSON_DATA))

        backends.json.load_results(tdir)


@utils.no_error
def test_load_results_folder():
    """ Test that load_results takes a folder with a file named results.json """
    with utils.tempdir() as tdir:
        with open(os.path.join(tdir, 'results.json'), 'w') as tfile:
            tfile.write(json.dumps(utils.JSON_DATA))

        backends.json.load_results(tdir)


@utils.no_error
def test_load_results_file():
    """ Test that load_results takes a file """
    with utils.resultfile() as tfile:
        backends.json.load_results(tfile.name)


def test_load_json():
    """backends.load(): Loads .json files."""
    with utils.tempdir() as tdir:
        filename = os.path.join(tdir, 'results.json')
        with open(filename, 'w') as f:
            json.dump(utils.JSON_DATA, f)

        result = backends.load(filename)

    nt.assert_is_instance(result, results.TestrunResult)
    nt.assert_in('sometest', result.tests)


def test_piglit_decoder():
    """backends.json.piglit_decoder: Works correctly"""
    test = json.loads('{"foo": {"result": "pass"}}',
                      object_hook=backends.json.piglit_decoder)
    nt.assert_is_instance(test['foo'], results.TestResult)
