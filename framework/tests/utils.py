# Copyright (c) 2014 Intel Coporation

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

""" Common helpers for framework tests

This module collects common tools that are needed in more than one test module
in a single place.

"""

from __future__ import print_function, absolute_import
import os
import sys
import shutil
import tempfile
import functools
import subprocess
from contextlib import contextmanager

try:
    import simplejson as json
except ImportError:
    import json
from nose.plugins.skip import SkipTest
import nose.tools as nt

import framework.results


__all__ = [
    'with_tempfile',
    'resultfile',
    'tempdir',
    'JSON_DATA'
]


JSON_DATA = {
    "options": {
        "profile": "tests/fake.py",
        "filter": [],
        "exclude_filter": []
    },
    "results_version": framework.results.CURRENT_JSON_VERSION,
    "name": "fake-tests",
    "lspci": "fake",
    "glxinfo": "fake",
    "tests": {
        "sometest": {
            "result": "pass",
            "time": 0.01
        }
    }
}


class UtilsException(Exception):
    """ An exception to be raised by utils """
    pass


@contextmanager
def resultfile():
    """ Create a stringio with some json in it and pass that as results """
    with tempfile.NamedTemporaryFile(delete=False) as output:
        json.dump(JSON_DATA, output)

    yield output

    os.remove(output.name)


@contextmanager
def with_tempfile(contents):
    """ Provides a context manager for a named tempfile

    This contextmanager creates a named tempfile, writes data into that
    tempfile, then closes it and yields the filepath. After the context is
    returned it closes and removes the tempfile.

    Arguments:
    contests -- This should be a string (unicode or str), in which case it is
                written directly into the file.

    """
    # Do not delete the tempfile as soon as it is closed
    temp = tempfile.NamedTemporaryFile(delete=False)
    temp.write(contents)
    temp.close()

    yield temp.name

    os.remove(temp.name)


@contextmanager
def tempdir():
    """ Creates a temporary directory, returns it, and then deletes it """
    tdir = tempfile.mkdtemp()
    yield tdir
    shutil.rmtree(tdir)


@nt.nottest
class GeneratedTestWrapper(object):
    """ An object proxy for nose test instances

    Nose uses python generators to create test generators, the drawback of this
    is that unless the generator is very specifically engineered it yeilds the
    same instance multiple times. Since nose uses an instance attribute to
    display the name of the test on a failure, and it prints the failure
    dialogue after the run is complete all failing tests from a single
    generator will end up with the name of the last test generated. This
    generator is used in conjunction with the nose_generator() decorator to
    create multiple objects each with a unique description attribute, working
    around the bug.
    Upstream bug: https://code.google.com/p/python-nose/issues/detail?id=244

    This uses functoos.update_wrapper to proxy the underlying object, and
    provides a __call__ method (which allows it to be called like a function)
    that calls the underling function.

    This class can also be used to wrap a class, but that class needs to
    provide a __call__ method, and use that to return results.

    Arguments:
    wrapped -- A function or function-like-class

    """
    def __init__(self, wrapped):
        self._wrapped = wrapped
        functools.update_wrapper(self, self._wrapped)

    def __call__(self, *args, **kwargs):
        """ calls the wrapped function

        Arguments:
        *args -- arguments to be passed to the wrapped function
        **kwargs -- keyword arguments to be passed to the wrapped function
        """
        return self._wrapped(*args, **kwargs)


def nose_generator(func):
    """ Decorator for nose test generators to us GeneratedTestWrapper

    This decorator replaces each function yeilded by a test generator with a
    GeneratedTestWrapper reverse-proxy object

    """
    def test_wrapper(*args, **kwargs):
        for x in func(*args, **kwargs):
            x = list(x)
            x[0] = GeneratedTestWrapper(x[0])
            yield tuple(x)  # This must be a tuple for some reason

    return test_wrapper


def privileged_test(func):
    """ Wrapper to name the tests as sudo

    This makes the name of the function contain sudo, which is useful for
    excluding tests with privileged execution requirements

    """
    def sudo_test_wrapper(*args, **kwargs):
        func(*args, **kwargs)

    return sudo_test_wrapper


class TestWithEnvClean(object):
    """ Class that does cleanup with saved state

    This could be done with test fixtures, but this should be cleaner in the
    specific case of cleaning up environment variables

    Nose will run a method (bound or unbound) at the start of the test called
    setup() and one at the end called teardown(), we have added a teardown
    method.

    Using this gives us the assurance that we're not relying on settings from
    other tests, making ours pass or fail, and that os.enviorn is the same
    going in as it is going out.

    This is modeled after Go's defer keyword.

    """
    def __init__(self):
        self._saved = set()
        self._teardown_calls = []

    def add_teardown(self, var, restore=True):
        """ Add os.environ values to remove in teardown """
        if var in os.environ:
            self._saved.add((var, os.environ.get(var), restore))
            del os.environ[var]

    def defer(self, func, *args):
        """ Add a function (with arguments) to be run durring cleanup """
        self._teardown_calls.append((func, args))

    def teardown(self):
        """ Teardown the test

        Restore any variables that were unset at the begining of the test, and
        run any differed methods.

        """
        for key, value, restore in self._saved:
            # If value is None the value was unset previously, put it back
            if value is None:
                del os.environ[key]
            elif restore:
                os.environ[key] = value

        # Teardown calls is a FIFO stack, the defered calls must be run in
        # reversed order to make any sense
        for call, args in reversed(self._teardown_calls):
            call(*args)


class StaticDirectory(object):
    """ Helper class providing shared files creation and cleanup

    One should override the setup_class method in a child class, call super(),
    and then add files to cls.dir.

    Tests in this class should NOT modify the contents of tidr, if you want
    that functionality you want a different class

    """
    @classmethod
    def setup_class(cls):
        """ Create a temperary directory that will be removed in teardown_class
        """
        cls.tdir = tempfile.mkdtemp()

    @classmethod
    def teardown_class(cls):
        """ Remove the temporary directory """
        shutil.rmtree(cls.tdir)


def binary_check(bin_):
    """Check for the existance of a binary or raise SkipTest."""
    with open(os.devnull, 'w') as null:
        try:
            subprocess.check_call(['which', bin_], stdout=null, stderr=null)
        except subprocess.CalledProcessError:
            raise SkipTest('Binary {} not available'.format(bin_))


def platform_check(plat):
    """If the platform is not in the list specified skip the test."""
    if not sys.platform.startswith(plat):
        raise SkipTest('Platform {} is not supported'.format(sys.platform))
