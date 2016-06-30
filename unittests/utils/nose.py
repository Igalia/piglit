# encoding=utf-8
# Copyright (c) 2014-2016 Intel Coporation

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

"""Nose extenders for framework tests.

This module collects useful tools that extend the functionality ro fix issues
in the nose suite. None of these features are piglit specific, they're all
generic.

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import errno
import functools
import importlib
import os
import shutil
import subprocess
import sys
import tempfile as tempfile_
from contextlib import contextmanager

from nose.plugins.skip import SkipTest
import six
import wrapt
try:
    from six.moves import getcwd
except ImportError:
    # pylint: disable=no-member
    if six.PY2:
        getcwd = os.getcwdu
    elif six.PY3:
        getcwd = os.getcwd
    # pylint: enable=no-member

from framework import compat

_WRITE_MODE = 'w'
_READ_MODE = 'r'


@compat.python_2_unicode_compatible
class TestFailure(AssertionError):
    """An exception to be raised when a test fails.

    Nose expects an AssertionError for test failures, so this is a sublcass of
    AssertionError.

    It provides the benefit of being able to take either a text message to
    print, or an exception instance. When passed text it will print the message
    exactly, when passed an exception it will print the exception type and the
    str() value of that exception.

    """
    def __init__(self, arg):
        super(TestFailure, self).__init__(self)
        self.__arg = arg

    def __str__(self):
        if isinstance(self.__arg, Exception):
            return u'exception type "{}" with message "{}" raised.'.format(
                type(self.__arg), str(self.__arg))
        else:
            return self.__arg


class UtilsError(Exception):
    """ An exception to be raised by utils """
    pass


class SentinalException(Exception):
    """An exception to be used as a sentinal."""
    pass


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
        cls.tdir = tempfile_.mkdtemp()

    @classmethod
    def teardown_class(cls):
        """ Remove the temporary directory """
        shutil.rmtree(cls.tdir)


class DocFormatter(object):
    """Decorator for formatting object docstrings.

    This class is designed to be initialized once per test module, and then one
    instance used as a decorator for all functions.

    Works as follows:
    >>> doc_formatter = DocFormatter({'format': 'foo', 'name': 'bar'})
    >>>
    >>> @doc_formatter
    ... def foo():
    ...     '''a docstring for {format} and {name}'''
    ...     pass
    ...
    >>> foo.__doc__
    'a docstring for foo and bar'

    This allows tests that can be dynamically updated by changing a single
    constant to have the test descriptions alos updated by the same constant.

    Arguments:
    table -- a dictionary of key value pairs to be converted

    """
    def __init__(self, table):
        self.__table = table

    def __call__(self, func):
        try:
            func.__doc__ = func.__doc__.format(**self.__table)
        except KeyError as e:
            # We want to catch this to ensure that a test that is looking for a
            # KeyError doesn't pass when it shouldn't
            raise UtilsError(e)

        return func


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


@contextmanager
def tempfile(contents):
    """ Provides a context manager for a named tempfile

    This contextmanager creates a named tempfile, writes data into that
    tempfile, then closes it and yields the filepath. After the context is
    returned it closes and removes the tempfile.

    Arguments:
    contests -- This should be a string (unicode or str), in which case it is
                written directly into the file.

    """
    # It is tempting to use NamedTemporaryFile here (the original
    # implementation did, in fact), but this won't work on windows beacuse of
    # implementation details. Since the goal isn't security anyway, just a
    # unique filename this is implemented in terms of tempdir.
    with tempdir() as t:
        name = os.path.join(t, 'tempfile')
        with open(name, mode=_WRITE_MODE) as f:
            f.write(contents)
        yield name


@contextmanager
def tempdir():
    """ Creates a temporary directory, returns it, and then deletes it """
    tdir = tempfile_.mkdtemp()
    try:
        yield tdir
    finally:
        shutil.rmtree(tdir)


def generator(func):
    """ Decorator for nose test generators to us GeneratedTestWrapper

    This decorator replaces each function yeilded by a test generator with a
    GeneratedTestWrapper reverse-proxy object

    """

    @functools.wraps(func)
    def test_wrapper(*args, **kwargs):
        for x in func(*args, **kwargs):
            x = list(x)
            x[0] = GeneratedTestWrapper(x[0])
            yield tuple(x)  # This must be a tuple for some reason

    return test_wrapper


@wrapt.decorator
def passthrough(wrapped, _, args, kwargs):
    return wrapped(*args, **kwargs)


class Skip(object):
    """Class providing conditional skipping support.

    Provides a number of class methods as alternate constructors for special
    skip conditions, these are merely convenience methods.

    """
    def __init__(self, condition, description):
        self.__skip = condition
        self.__description = description

    @classmethod
    def py3(cls, _=None):
        """Skip if the interpreter python 3."""
        return cls(six.PY3, 'Test is not relevant on python 3.x')

    @classmethod
    def py2(cls, _=None):
        """Skip if the interpreter is python 2."""
        return cls(six.PY2, 'Test is not relevant on python 2.x')

    @classmethod
    def module(cls, name, available=False):
        """Skip if the provided external module is (not) available."""
        assert isinstance(available, bool), 'avilable must be bool'
        def check():
            try:
                importlib.import_module(name)
            except ImportError:
                return False
            return True

        return cls(check() is not available,
                   'Test requires that module {} is {}available'.format(
                       name, '' if available else 'not '))

    @classmethod
    def backport(cls, version, name):
        """Skip if the interpreter needs a backport that isn't available.

        Arguments:
        version -- The minimum version that doesn't require the backport
        name -- the name of the required module

        """
        assert isinstance(version, float), 'version must be float'
        if float('.'.join(str(v) for v in sys.version_info[:2])) < version:
            return cls.module(name, True)
        return passthrough

    @classmethod
    def binary(cls, name):
        """Skip if the requested binary isn't available."""
        def check():
            with open(os.devnull, 'w') as null:
                try:
                    # Pass the bogus arg in case the program tries to read
                    # stdin, like xz
                    subprocess.check_call([name, 'totallymadeupdoesntexistarg'],
                                          stdout=null, stderr=null)
                except OSError as e:
                    if e.errno == errno.ENOENT:
                        return True
                except subprocess.CalledProcessError as e:
                    pass
            return False

        return cls(
            check(), 'Test requires that binary {} is available'.format(name))

    @classmethod
    def platform(cls, name, is_=False):
        return cls(sys.platform.startswith(name) is is_,
                   'Platform {} is not supported'.format(sys.platform))

    @wrapt.decorator
    def __call__(self, wrapped, instance, args, kwargs):
        if self.__skip:
            raise SkipTest(self.__description)
        return wrapped(*args, **kwargs)


def test_in_tempdir(func):
    """Decorator that moves to a new directory to run a test.

    This decorator ensures that the test moves to a new directory, and then
    returns to the original directory after the test completes.

    """
    @functools.wraps(func)
    def wrapper(*args, **kwargs):
        with tempdir() as tdir:
            with chdir(tdir):
                return func(*args, **kwargs)

    return wrapper


def not_raises(exceptions):
    """Decorator that causes a test to fail if it raises an exception.

    Without arguments this will raise a TestFailure if Exception is raised.
    arguments are passed those will be handed directly to the except keyword,
    and a TestFailure will be raised only for those exceptions.

    Uncaught exceptions will still be handled by nose and return an error.

    """

    def _wrapper(func):
        """wrapper that wraps the actual functiion definition."""

        @functools.wraps(func)
        def _inner(*args, **kwargs):
            """Wrapper for the function call."""
            try:
                func(*args, **kwargs)
            except exceptions as e:
                raise TestFailure(e)

        return _inner

    return _wrapper


def no_error(func):
    """Convenience wrapper around not_raises when any error is an exception

    """
    return not_raises(Exception)(func)


def capture_stderr(func):
    """Redirect stderr to stdout for nose capture.

    It would probably be better to implement a full stderr handler as a
    plugin...

    """
    @functools.wraps(func)
    def _inner(*args, **kwargs):
        restore = sys.stderr
        sys.stderr = sys.stdout
        try:
            func(*args, **kwargs)
        finally:
            sys.stderr = restore

    return _inner


@contextmanager
def chdir(goto):
    """chdir to a directory and back, guaranteed.

    This contextmanager ensures that after changing directory you cahgne back,
    using a try/finally block.

    """
    returnto = getcwd()
    os.chdir(goto)
    try:
        yield
    finally:
        os.chdir(returnto)
