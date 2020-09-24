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

""" Tests for the backend package """

import pytest

from framework import backends
from framework import results

# pylint: disable=no-self-use



# Helpers


@pytest.yield_fixture
def mock_backend(mocker):
    """Add an extra backend for testing."""
    mocker.patch.dict(
        backends.BACKENDS,
        {'test_backend': backends.register.Registry(
            extensions=['.test_backend'],
            backend=None,
            load=lambda x, _: x,
            meta=None,
            write=None,
        )})
    yield


# Tests


class TestGetBackend(object):
    """Tests for the get_backend function."""

    @pytest.mark.parametrize("name,expected", [
        ('json', backends.json.JSONBackend),
        ('junit', backends.junit.JUnitBackend),
    ])
    def test_basic(self, name, expected):
        """Test that ensures the expected input and output."""
        assert backends.get_backend(name) is expected

    def test_unknown_backend(self):
        """backends.get_backend: An error is raised with an unknown backend."""
        with pytest.raises(backends.BackendError):
            backends.get_backend('obviously fake backend')

    def test_notimplemented(self, mock_backend):  # pylint: disable=redefined-outer-name,unused-argument
        """backends.get_backend: An error is raised if a backend isn't
        implemented.
        """
        with pytest.raises(backends.BackendNotImplementedError):
            backends.get_backend('test_backend')


class TestLoad(object):
    """Tests for the load function."""

    def test_basic(self, mocker, tmpdir):  # pylint: disable=unused-argument
        """backends.load: works as expected.

        This is an interesting function to test, because it is just a wrapper
        that returns a TestrunResult object. So most of the testing should be
        happening in the tests for each backend.

        However, we can test this by injecting a fake backend, and ensuring
        that we get back what we expect. What we do is inject list(), which
        means that we should get back [file_path], instead of just file_path,
        like the legitimate backends return.
        """
        mocker.patch.dict(
            backends.BACKENDS,
            {'test_backend': backends.register.Registry(
                extensions=['.test_backend'],
                backend=None,
                load=lambda x, _: [x],
                meta=None,
                write=None,
            )})

        p = tmpdir.join('foo.test_backend')
        p.write('foo')
        test = backends.load(str(p))
        assert [str(p)] == test

    def test_unknown(self, tmpdir):
        p = tmpdir.join('foo.test_extension')
        p.write('foo')

        with pytest.raises(backends.BackendError):
            backends.load(str(p))

    def test_interrupted(self, tmpdir, mock_backend):  # pylint: disable=unused-argument,redefined-outer-name
        """backends.load: works for resuming (no extension known)."""
        tmpdir.mkdir('tests')
        with tmpdir.join('tests', '0.test_backend').open('w') as f:
            f.write('foo')

        backends.load(str(tmpdir))

    def test_notimplemented(self, tmpdir, mocker):
        """backends.load(): An error is raised if a loader isn't properly
        implemented.
        """
        mocker.patch.dict(
            backends.BACKENDS,
            {'test_backend': backends.register.Registry(
                extensions=['.test_backend'],
                backend=None,
                load=None,
                meta=None,
                write=None,
            )})
        p = tmpdir.join('foo.test_backend')
        p.write('foo')

        with pytest.raises(backends.BackendNotImplementedError):
            backends.load(str(p))

    def test_trailing_dot(self, mocker):
        """framework.backends.load: handles the result name ending in '.'.

        Basically if this reaches a BackendNotImplementedError, then the '.'
        was handled correctly, otherwise if it's '.' then we should reach the
        BackendError, which is incorrect.
        """
        mocker.patch.dict(
            backends.BACKENDS,
            {'test_backend': backends.register.Registry(
                extensions=['.test_backend'],
                backend=None,
                load=None,
                meta=None,
                write=None,
            )})
        with pytest.raises(backends.BackendNotImplementedError):
            backends.load('foo.test_backend..gz')

    def test_old(self, tmpdir, mock_backend):  # pylint: disable=unused-argument,redefined-outer-name
        """backends.load: Ignores files ending in '.old'.

        If this raises a BackendError it means it didn't find a backend to use,
        thus it skipped the file ending in '.old'.
        """
        tmpdir.mkdir('test')
        p = tmpdir.join('test')
        with p.join('results.test_backend.old').open('w') as f:
            f.write('foo')

        with pytest.raises(backends.BackendError):
            backends.load(str(p))


class TestWrite(object):
    """Tests for the write function."""

    @classmethod
    def setup_class(cls):
        """class fixture."""
        res = results.TestrunResult()
        res.tests['foo'] = results.TestResult('pass')
        res.tests['bar'] = results.TestResult('fail')
        res.tests['oink'] = results.TestResult('crash')
        res.tests['bonk'] = results.TestResult('warn')
        res.tests['bor'] = results.TestResult('crash')
        res.tests['bor'].subtests['1'] = 'pass'
        res.tests['bor'].subtests['2'] = 'skip'
        res.tests['bor'].subtests['3'] = 'fail'
        res.tests['bor'].subtests['4'] = 'pass'

        cls.results = res

    @pytest.mark.parametrize('expected', [
        (True),
        (False),
    ])
    def test_basic(self, expected, mocker):
        """backends.write: works as expected.

        This is an interesting function to test, because it is just a wrapper
        that returns True or False. So most of the testing should be
        happening in the tests for each backend.

        However, we can test this by injecting a fake backend, and ensuring
        that we get back what we expect. What we do is inject list(), which
        means that we should get back [file_path], instead of just file_path,
        like the legitimate backends return.
        """
        mocker.patch.dict(
            backends.BACKENDS,
            {'test_backend': backends.register.Registry(
                extensions=['.test_backend'],
                backend=None,
                load=None,
                meta=None,
                write=lambda x, _: expected,
            )})

        test = backends.write(self.results, 'foo.test_backend')
        assert expected == test

    @pytest.mark.raises(exception=backends.BackendError)
    def test_unknown(self, tmpdir):  # pylint: disable=unused-argument
        backends.write(self.results, 'foo.test_extension')

    @pytest.mark.raises(exception=backends.BackendNotImplementedError)
    def test_notimplemented(self, mocker):
        """backends.write(): An error is raised if a writer isn't properly
        implemented.
        """
        mocker.patch.dict(
            backends.BACKENDS,
            {'test_backend': backends.register.Registry(
                extensions=['.test_backend'],
                backend=None,
                load=None,
                meta=None,
                write=None,
            )})

        backends.write(self.results, 'foo.test_backend')

    @pytest.mark.raises(exception=backends.BackendNotImplementedError)
    def test_trailing_dot(self, mocker):
        """framework.backends.write: handles the result name ending in '.'.

        Basically if this reaches a BackendNotImplementedError, then the '.'
        was handled correctly, otherwise if it's '.' then we should reach the
        BackendError, which is incorrect.
        """
        mocker.patch.dict(
            backends.BACKENDS,
            {'test_backend': backends.register.Registry(
                extensions=['.test_backend'],
                backend=None,
                load=None,
                meta=None,
                write=None,
            )})

        backends.write(self.results, 'foo.test_backend..gz')

    @pytest.mark.raises(exception=backends.BackendError)
    def test_old(self, mock_backend):
        """backends.write: Ignores files ending in '.old'.

        If this raises a BackendError it means it didn't find a backend to use,
        thus it skipped the file ending in '.old'.
        """
        backends.write(self.results, 'results.test_backend.old')


class TestSetMeta(object):
    """Tests for the set_meta function."""

    def test_basic(self, mocker):
        """Basic argument/output test."""
        mocker.patch.dict(
            backends.BACKENDS,
            {'test_backend': backends.register.Registry(
                extensions=['.test_backend'],
                backend=None,
                load=None,
                meta=lambda x: x.append('bar'),
                write=None,
            )})

        test = []
        backends.set_meta('test_backend', test)
        assert test == ['bar']

    def test_no_backened(self):
        """backends.set_meta: raises an error if there is no meta function."""
        with pytest.raises(backends.BackendError):
            backends.set_meta('foo', {})

    def test_notimplemented(self, mock_backend):  # pylint: disable=redefined-outer-name,unused-argument
        """backends.load(): An error is raised if a set_meta isn't properly
        implmented.
        """
        with pytest.raises(backends.BackendNotImplementedError):
            backends.set_meta('test_backend', {})
