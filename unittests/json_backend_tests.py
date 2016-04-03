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

# pylint: disable=missing-docstring,protected-access

""" Tests for the backend package """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os

try:
    import simplejson as json
except ImportError:
    import json
import nose.tools as nt

from framework import results, backends, exceptions, grouptools
from . import utils
from .backends_tests import BACKEND_INITIAL_META


def setup_module():
    utils.set_compression('none')


def teardown_module():
    utils.unset_compression()


def test_initialize_jsonbackend():
    """backends.json.JSONBackend: Class initializes

    This needs to be handled separately from the others because it requires
    arguments

    """
    with utils.tempdir() as tdir:
        func = backends.json.JSONBackend(tdir)
        nt.ok_(isinstance(func, backends.json.JSONBackend))


def test_json_initialize_metadata():
    """backends.json.JSONBackend.initialize(): produces a metadata.json file"""
    with utils.tempdir() as f:
        test = backends.json.JSONBackend(f)
        test.initialize(BACKEND_INITIAL_META)

        nt.ok_(os.path.exists(os.path.join(f, 'metadata.json')))


class TestJSONTestMethod(utils.StaticDirectory):
    @classmethod
    def setup_class(cls):
        cls.test_name = grouptools.join('a', 'test', 'group', 'test1')
        cls.result = results.TestResult()
        cls.time = 1.2345
        cls.result = 'pass'
        cls.out = 'this is stdout'
        cls.err = 'this is stderr'

        super(TestJSONTestMethod, cls).setup_class()
        test = backends.json.JSONBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        with test.write_test(cls.test_name) as t:
            t(cls.result)

    def test_write_test(self):
        """backends.json.JSONBackend.write_test(): adds tests to a 'tests' directory"""
        nt.ok_(os.path.exists(os.path.join(self.tdir, 'tests', '0.json')))

    @utils.no_error
    def test_json_is_valid(self):
        """backends.json.JSONBackend.write_test(): produces valid json"""
        with open(os.path.join(self.tdir, 'tests', '0.json'), 'r') as f:
            json.load(f)

    def test_json_is_correct(self):
        """backends.json.JSONBackend.write_test(): produces correct json"""
        with open(os.path.join(self.tdir, 'tests', '0.json'), 'r') as f:
            test = json.load(f)

        nt.assert_dict_equal({self.test_name: self.result}, test)


class TestJSONTestFinalize(utils.StaticDirectory):
    # We're explictely setting none here since the default can change from none
    @classmethod
    def setup_class(cls):
        cls.test_name = grouptools.join('a', 'test', 'group', 'test1')
        cls.result = results.TestResult('pass')

        super(TestJSONTestFinalize, cls).setup_class()
        test = backends.json.JSONBackend(cls.tdir)
        test.initialize(BACKEND_INITIAL_META)
        with test.write_test(cls.test_name) as t:
            t(cls.result)
        test.finalize()

    def test_remove_metadata(self):
        """backends.json.JSONBackend.finalize(): removes metadata.json"""
        nt.ok_(not os.path.exists(os.path.join(self.tdir, 'metadata.json')))

    def test_remove_tests(self):
        """backends.json.JSONBackend.finalize(): removes tests directory"""
        nt.ok_(not os.path.exists(os.path.join(self.tdir, 'tests')))

    def test_create_results(self):
        """backends.json.JSONBackend.finalize(): creates a results.json file
        """
        nt.ok_(os.path.exists(os.path.join(self.tdir, 'results.json')))

    @utils.no_error
    def test_results_valid(self):
        """backends.json.JSONBackend.finalize(): results.json is valid"""
        with open(os.path.join(self.tdir, 'results.json'), 'r') as f:
            json.load(f)


def test_update_results_current():
    """backends.json.update_results(): returns early when the results_version is current"""
    data = utils.JSON_DATA.copy()
    data['results_version'] = backends.json.CURRENT_JSON_VERSION

    with utils.tempdir() as d:
        with open(os.path.join(d, 'main'), 'w') as f:
            json.dump(data, f, default=backends.json.piglit_encoder)

        with open(os.path.join(d, 'main'), 'r') as f:
            base = backends.json._load(f)

        res = backends.json._update_results(base, f.name)

    nt.assert_dict_equal(res.__dict__, base.__dict__)


def test_update_results_old():
    """backends.json.update_results(): updates results

    Because of the design of the our updates (namely that they silently
    incrementally update from x to y) it's impossible to know exactly what
    we'll get at th end without having tests that have to be reworked each time
    updates are run. Since there already is (at least for v0 -> v1) a fairly
    comprehensive set of tests, this test only tests that update_results() has
    been set equal to the CURRENT_JSON_VERSION, (which is one of the effects of
    running update_results() with the assumption that there is sufficient other
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


@nt.raises(AssertionError)
def test_json_resume_non_folder():
    """backends.json._resume: doesn't accept a file"""
    with utils.tempfile('') as f:
        backends.json._resume(f)


def test_resume_load_valid():
    """backends.json._resume: loads valid results"""
    with utils.tempdir() as f:
        backend = backends.json.JSONBackend(f)
        backend.initialize(BACKEND_INITIAL_META)
        with backend.write_test("group1/test1") as t:
            t(results.TestResult('fail'))
        with backend.write_test("group1/test2") as t:
            t(results.TestResult('pass'))
        with backend.write_test("group2/test3") as t:
            t(results.TestResult('fail'))

        test = backends.json._resume(f)

        nt.assert_set_equal(
            set(test.tests.keys()),
            set(['group1/test1', 'group1/test2', 'group2/test3']),
        )


def test_resume_load_invalid():
    """backends.json._resume: ignores invalid results"""
    with utils.tempdir() as f:
        backend = backends.json.JSONBackend(f)
        backend.initialize(BACKEND_INITIAL_META)
        with backend.write_test("group1/test1") as t:
            t(results.TestResult('fail'))
        with backend.write_test("group1/test2") as t:
            t(results.TestResult('pass'))
        with backend.write_test("group2/test3") as t:
            t(results.TestResult('fail'))
        with open(os.path.join(f, 'tests', 'x.json'), 'w') as w:
            w.write('foo')

        test = backends.json._resume(f)

        nt.assert_set_equal(
            set(test.tests.keys()),
            set(['group1/test1', 'group1/test2', 'group2/test3']),
        )


def test_resume_load_incomplete():
    """backends.json._resume: loads incomplete results.

    Because resume, aggregate, and summary all use the function called _resume
    we can't remove incomplete tests here. It's probably worth doing a refactor
    to split some code out and allow this to be done in the resume path.

    """
    with utils.tempdir() as f:
        backend = backends.json.JSONBackend(f)
        backend.initialize(BACKEND_INITIAL_META)
        with backend.write_test("group1/test1") as t:
            t(results.TestResult('fail'))
        with backend.write_test("group1/test2") as t:
            t(results.TestResult('pass'))
        with backend.write_test("group2/test3") as t:
            t(results.TestResult('crash'))
        with backend.write_test("group2/test4") as t:
            t(results.TestResult('incomplete'))

        test = backends.json._resume(f)

        nt.assert_set_equal(
            set(test.tests.keys()),
            set(['group1/test1', 'group1/test2', 'group2/test3',
                 'group2/test4']),
        )


@utils.no_error
def test_load_results_folder_as_main():
    """backends.json.load_results: takes a folder with a file named main in it
    """
    with utils.tempdir() as tdir:
        with open(os.path.join(tdir, 'main'), 'w') as tfile:
            tfile.write(json.dumps(utils.JSON_DATA,
                                   default=backends.json.piglit_encoder))

        backends.json.load_results(tdir, 'none')


@utils.no_error
def test_load_results_folder():
    """backends.json.load_results: takes a folder with a file named results.json"""
    with utils.tempdir() as tdir:
        with open(os.path.join(tdir, 'results.json'), 'w') as tfile:
            tfile.write(json.dumps(utils.JSON_DATA,
                                   default=backends.json.piglit_encoder))

        backends.json.load_results(tdir, 'none')


@utils.no_error
def test_load_results_file():
    """backends.json.load_results: Loads a file passed by name"""
    with utils.resultfile() as tfile:
        backends.json.load_results(tfile.name, 'none')


def test_load_json():
    """backends.load(): Loads .json files."""
    with utils.tempdir() as tdir:
        filename = os.path.join(tdir, 'results.json')
        with open(filename, 'w') as f:
            json.dump(utils.JSON_DATA, f, default=backends.json.piglit_encoder)

        result = backends.load(filename)

    nt.assert_is_instance(result, results.TestrunResult)
    nt.assert_in('sometest', result.tests)


def test_piglit_decoder_result():
    """backends.json.piglit_decoder: turns results into TestResults"""
    test = json.loads('{"foo": {"result": "pass", "__type__": "TestResult"}}',
                      object_hook=backends.json.piglit_decoder)
    nt.assert_is_instance(test['foo'], results.TestResult)


def test_piglit_decoder_old_result():
    """backends.json.piglit_decoder: does not turn old results into TestResults
    """
    test = json.loads('{"foo": {"result": "pass"}}',
                      object_hook=backends.json.piglit_decoder)
    nt.assert_is_instance(test['foo'], dict)


@nt.raises(exceptions.PiglitFatalError)
def test_load_bad_json():
    """backends.json._load: Raises fatal error if json is corrupt"""
    with utils.tempfile('{"bad json": }') as f:
        with open(f, 'r') as tfile:
            backends.json._load(tfile)
