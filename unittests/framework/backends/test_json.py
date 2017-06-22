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

""" Tests for the backend package """

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
try:
    import simplejson as json
except ImportError:
    import json
try:
    import mock
except ImportError:
    from unittest import mock

import jsonschema
import pytest
import six

from framework import backends
from framework import exceptions
from framework import grouptools
from framework import results

from . import shared

# pylint: disable=no-self-use,protected-access

SCHEMA = os.path.join(os.path.dirname(__file__), 'schema',
                      'piglit-{}.json'.format(backends.json.CURRENT_JSON_VERSION))


@pytest.yield_fixture(scope='module', autouse=True)
def mock_compression():
    with mock.patch.dict(backends.json.compression.os.environ,
                         {'PIGLIT_COMPRESSION': 'none'}):
        yield


class TestJSONBackend(object):
    """Tests for the JSONBackend class."""

    class TestInitialize(object):
        """Tests for the initialize method."""

        def test_metadata_file_created(self, tmpdir):
            p = six.text_type(tmpdir)
            test = backends.json.JSONBackend(p)
            test.initialize(shared.INITIAL_METADATA)
            assert os.path.exists(os.path.join(p, 'metadata.json'))

    class TestWriteTest(object):
        """Tests for the write_test method."""

        def test_write(self, tmpdir):
            """The write method should create a file."""
            p = six.text_type(tmpdir)
            test = backends.json.JSONBackend(p)
            test.initialize(shared.INITIAL_METADATA)

            with test.write_test('bar') as t:
                t(results.TestResult())

            assert tmpdir.join('tests/0.json').check()

        def test_load(self, tmpdir):
            """Test that the written JSON can be loaded.

            This doesn't attempt to validate the schema of the code (That is
            handled elsewhere), instead it just attempts a touch test of "can
            this be read as JSON".
            """
            p = six.text_type(tmpdir)
            test = backends.json.JSONBackend(p)
            test.initialize(shared.INITIAL_METADATA)

            with test.write_test('bar') as t:
                t(results.TestResult())

            with tmpdir.join('tests/0.json').open('r') as f:
                json.load(f)

    class TestFinalize(object):
        """Tests for the finalize method."""

        name = grouptools.join('a', 'test', 'group', 'test1')
        result = results.TestResult('pass')

        @pytest.fixture(scope='class')
        def result_dir(self, tmpdir_factory):
            directory = tmpdir_factory.mktemp('main')
            test = backends.json.JSONBackend(six.text_type(directory))
            test.initialize(shared.INITIAL_METADATA)
            with test.write_test(self.name) as t:
                t(self.result)
            test.finalize(
                {'time_elapsed':
                    results.TimeAttribute(start=0.0, end=1.0).to_json()})

            return directory

        def test_metadata_removed(self, result_dir):
            assert not result_dir.join('metadata.json').check()

        def test_tests_directory_removed(self, result_dir):
            assert not result_dir.join('tests').check()

        def test_results_file_created(self, result_dir):
            # Normally this would also have a compression extension, but this
            # module has a setup fixture that forces the compression to None.
            assert result_dir.join('results.json').check()

        def test_results_are_json(self, result_dir):
            # This only checks that the output is valid JSON, not that the
            # schema is correct
            with result_dir.join('results.json').open('r') as f:
                json.load(f)

        def test_results_are_valid(self, result_dir):
            """Test that the values produced are valid."""
            with result_dir.join('results.json').open('r') as f:
                json_ = json.load(f)

            with open(SCHEMA, 'r') as f:
                schema = json.load(f)

            jsonschema.validate(json_, schema)


class TestUpdateResults(object):
    """Test for the _update_results function."""

    def test_current_version(self, tmpdir, mocker):
        """backends.json.update_results(): returns early when the
        results_version is current.
        """
        class Sentinel(Exception):
            pass

        mocker.patch('framework.backends.json.os.rename',
                     mocker.Mock(side_effect=Sentinel))
        p = tmpdir.join('results.json')
        p.write(json.dumps(shared.JSON))

        with p.open('r') as f:
            base = backends.json._load(f)
        backends.json._update_results(base, six.text_type(p))


class TestResume(object):
    """tests for the resume function."""

    def test_load_file(self, tmpdir):
        p = tmpdir.join('results.json')
        p.write('')

        with pytest.raises(AssertionError):
            backends.json._resume(six.text_type(p))

    def test_load_valid_folder(self, tmpdir):
        """backends.json._resume: loads valid results."""
        backend = backends.json.JSONBackend(six.text_type(tmpdir))
        backend.initialize(shared.INITIAL_METADATA)
        with backend.write_test("group1/test1") as t:
            t(results.TestResult('fail'))
        with backend.write_test("group1/test2") as t:
            t(results.TestResult('pass'))
        with backend.write_test("group2/test3") as t:
            t(results.TestResult('fail'))
        test = backends.json._resume(six.text_type(tmpdir))

        assert set(test.tests.keys()) == \
            {'group1/test1', 'group1/test2', 'group2/test3'}

    @pytest.mark.xfail
    def test_load_invalid_folder(self, tmpdir):
        """backends.json._resume: ignores invalid results"""
        # XXX: I'm not sure if this test is worth fixing or not, it would
        # involve a lot of code, and for this case to actually be tripped a
        # user would have to write a file into the tests directory that isn't a
        # number
        f = six.text_type(tmpdir)
        backend = backends.json.JSONBackend(f)
        backend.initialize(shared.INITIAL_METADATA)
        with backend.write_test("group1/test1") as t:
            t(results.TestResult('fail'))
        with backend.write_test("group1/test2") as t:
            t(results.TestResult('pass'))
        with backend.write_test("group2/test3") as t:
            t(results.TestResult('fail'))
        with open(os.path.join(f, 'tests', 'x.json'), 'w') as w:
            w.write('foo')
        test = backends.json._resume(f)

        assert set(test.tests.keys()) == \
            {'group1/test1', 'group1/test2', 'group2/test3'}

    def test_load_incomplete(self, tmpdir):
        """backends.json._resume: loads incomplete results.

        Because resume, aggregate, and summary all use the function called
        _resume we can't remove incomplete tests here. It's probably worth
        doing a refactor to split some code out and allow this to be done in
        the resume path.
        """
        f = six.text_type(tmpdir)
        backend = backends.json.JSONBackend(f)
        backend.initialize(shared.INITIAL_METADATA)
        with backend.write_test("group1/test1") as t:
            t(results.TestResult('fail'))
        with backend.write_test("group1/test2") as t:
            t(results.TestResult('pass'))
        with backend.write_test("group2/test3") as t:
            t(results.TestResult('crash'))
        with backend.write_test("group2/test4") as t:
            t(results.TestResult('incomplete'))
        test = backends.json._resume(f)

        assert set(test.tests.keys()) == \
            {'group1/test1', 'group1/test2', 'group2/test3', 'group2/test4'}


class TestLoadResults(object):
    """Tests for the load_results function."""

    def test_folder_with_results_json(self, tmpdir):
        """backends.json.load_results: takes a folder with a file named
        results.json.
        """
        p = tmpdir.join('results.json')
        with p.open('w') as f:
            f.write(json.dumps(shared.JSON))
        backends.json.load_results(six.text_type(tmpdir), 'none')

    def test_load_file(self, tmpdir):
        """backends.json.load_results: Loads a file passed by name"""
        p = tmpdir.join('my file')
        with p.open('w') as f:
            f.write(json.dumps(shared.JSON))
        backends.json.load_results(six.text_type(p), 'none')

    def test_inst(self, tmpdir):
        p = tmpdir.join('my file')
        with p.open('w') as f:
            f.write(json.dumps(shared.JSON))
        assert isinstance(backends.json.load_results(six.text_type(p), 'none'),
                          results.TestrunResult)


class TestLoad(object):
    """Tests for the _load function."""

    def test_load_bad_json(self, tmpdir):
        """backends.json._load: Raises fatal error if json is corrupt"""
        p = tmpdir.join('foo')
        p.write('{"bad json": }')
        with p.open('r') as f:
            with pytest.raises(exceptions.PiglitFatalError):
                backends.json._load(f)
