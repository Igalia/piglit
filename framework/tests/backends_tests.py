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

import nose.tools as nt

from framework import core, backends
import framework.tests.utils as utils


BACKEND_INITIAL_META = {
    'name': 'name',
    'test_count': 0,
    'env': {},
    'options': {k: v for k, v in core.Options()},
}


# Helpers

def _notimplemented_setup():
    """Setup function that injects a new test Registry into the BACKENDS
    variable.

    should be used in conjunction with the _registry_teardown method.

    """
    backends.BACKENDS['test_backend'] = backends.register.Registry(
        extensions=['.test_backend'],
        backend=None,
        load=None,
        meta=None,
    )


def _registry_teardown():
    """Remove the test_backend Register from backends.BACKENDS."""
    del backends.BACKENDS['test_backend']


# Tests


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
        check.description = \
            'backends.get_backend({0}): returns {0} backend'.format(name)
        yield check, name, inst


@nt.raises(backends.BackendError)
def test_get_backend_unknown():
    """backends.get_backend: An error is raised with an unkown backend."""
    backends.get_backend('obviously fake backend')


@nt.raises(backends.BackendNotImplementedError)
@nt.with_setup(_notimplemented_setup, _registry_teardown)
def test_get_backend_notimplemented():
    """backends.get_backend: An error is raised if a backend isn't implemented.
    """
    backends.get_backend('test_backend')


@nt.with_setup(teardown=_registry_teardown)
@utils.test_in_tempdir
def test_load():
    """backends.load(): works as expected.

    This is an interesting function to test, because it is just a wrapper that
    returns a TestrunResult object. So most of the testing should be happening
    in the tests for each backend.

    However, we can test this by injecting a fake backend, and ensuring that we
    get back what we expect. What we do is inject list(), which menas that we
    should get back [file_path].

    """
    backends.BACKENDS['test_backend'] = backends.register.Registry(
        extensions=['.test_extension'],
        backend=None,
        load=lambda x: [x],
        meta=None,
    )

    file_path = 'foo.test_extension'
    with open(file_path, 'w') as f:
        f.write('foo')

    test = backends.load(file_path)
    nt.assert_list_equal([file_path], test)


@nt.raises(backends.BackendError)
@utils.test_in_tempdir
def test_load_unknown():
    """backends.load(): An error is raised if no modules supportes `extension`
    """
    file_path = 'foo.test_extension'

    with open(file_path, 'w') as f:
        f.write('foo')
    backends.load(file_path)


@utils.no_error
@nt.with_setup(_notimplemented_setup, _registry_teardown)
@utils.test_in_tempdir
def test_load_resume():
    """backends.load: works for resuming (no extension known)."""
    backends.BACKENDS['test_backend'] = backends.register.Registry(
        extensions=['.test_backend'],
        backend=None,
        load=lambda x: x,
        meta=None,
    )
    os.mkdir('tests')
    name = os.path.join('tests', '0.test_backend')
    with open(name, 'w') as f:
        f.write('foo')

    backends.load('.')


@nt.raises(backends.BackendNotImplementedError)
@nt.with_setup(_notimplemented_setup, _registry_teardown)
@utils.test_in_tempdir
def test_load_notimplemented():
    """backends.load(): An error is raised if a loader isn't properly implmented.
    """
    file_path = 'foo.test_backend'
    with open(file_path, 'w') as f:
        f.write('foo')

    backends.load(file_path)


def test_set_meta():
    """backends.set_meta(): Works as expected."""
    backends.BACKENDS['test_backend'] = backends.register.Registry(
        extensions=None,
        backend=None,
        load=None,
        meta=lambda x: x.append('bar'),
    )

    test = ['foo']

    backends.set_meta('test_backend', test)
    nt.assert_list_equal(test, ['foo', 'bar'])


@nt.raises(backends.BackendError)
def test_set_meta_no_backened():
    """backends.set_meta: raises an error if there is no meta function."""
    backends.set_meta('foo', {})


@nt.raises(backends.BackendNotImplementedError)
@nt.with_setup(_notimplemented_setup, _registry_teardown)
def test_set_meta_notimplemented():
    """backends.load(): An error is raised if a set_meta isn't properly implmented.
    """
    backends.set_meta('test_backend', {})
