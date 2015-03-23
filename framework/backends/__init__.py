# Copyright (c) 2014, 2015 Intel Corporation

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

"""Provides a module like interface for backends.

This package provides an abstract interface for working with backends, which
implement various functions as provided in the Registry class, and then provide
a Registry instance as REGISTRY, which maps individual objects to objects that
piglit expects to use. This module then provides a thin abstraction layer so
that piglit is never aware of what backend it's using, it just asks for an
object and receives one.

Most consumers will want to import framework.backends and work directly with
the helper functions here. For some more detailed uses (test cases expecially)
the modules themselves may be used.

When this module is loaded it will search through framework/backends for python
modules (those ending in .py), and attempt to import them. Then it will look
for an attribute REGISTRY in those modules, and if it as a
framework.register.Registry instance, it will add the name of that module (sans
.py) as a key, and the instance as a value to the BACKENDS dictionary. Each of
the helper functions in this module uses that dictionary to find the function
that a user actually wants.

"""

import os
import importlib

from .register import Registry

__all__ = [
    'BACKENDS',
    'BackendError',
    'BackendNotImplementedError',
    'get_backend',
    'load',
    'set_meta',
]


class BackendError(Exception):
    pass


class BackendNotImplementedError(NotImplementedError):
    pass


def _register():
    """Register backends.

    Walk through the list of backends and register them to a name in a
    dictionary, so that they can be referenced from helper functions.
    
    """
    registry = {}

    for module in os.listdir(os.path.dirname(os.path.abspath(__file__))):
        module, extension = os.path.splitext(module)
        if extension == '.py':
            mod = importlib.import_module('framework.backends.{}'.format(module))
            if hasattr(mod, 'REGISTRY') and isinstance(mod.REGISTRY, Registry):
                registry[module] = mod.REGISTRY

    return registry


BACKENDS = _register()


def get_backend(backend):
    """Returns a BackendInstance based on the string passed.

    If the backend isn't a known module, then a BackendError will be raised, it
    is the responsibility of the caller to handle this error.

    If the backend module exists, but there is not active implementation then a
    BackendNotImplementedError will be raised, it is also the responsiblity of
    the caller to handle this error.
    
    """
    try:
        inst = BACKENDS[backend].backend
    except KeyError:
        raise BackendError('Unknown backend: {}'.format(backend))

    if inst is None:
        raise BackendNotImplementedError(
            'Backend for {} is not implemented'.format(backend))

    return inst


def load(file_path):
    """Wrapper for loading runs.

    This function will attempt to determine how to load the file (based on file
    extension), and then pass the file path into the appropriate loader, and
    then return the TestrunResult instance.

    """
    extension = None

    if os.path.isfile(file_path):
        extension = os.path.splitext(file_path)[1]
        if not extension:
            extension = ''
    else:
        for file in os.listdir(file_path):
            if file.startswith('result'):
                extension = os.path.splitext(file)[1]
                break
            elif file == 'main':
                extension = ''
                break
    tests = os.path.join(file_path, 'tests')
    if extension is None:
        if os.path.exists(tests):
            extension = os.path.splitext(os.listdir(tests)[0])[1]
        else:
            # At this point we have failed to find any sort of backend, just except
            # and die
            raise BackendError("No backend found for any file in {}".format(
                file_path))

    for backend in BACKENDS.itervalues():
        if extension in backend.extensions:
            loader = backend.load
            break
    else:
        raise BackendError(
            'No module supports file extensions "{}"'.format(extension))

    if loader is None:
        raise BackendNotImplementedError(
            'Loader for {} is not implemented'.format(extension))

    return loader(file_path)


def set_meta(backend, result):
    """Wrapper around meta that gets the right meta function."""
    try:
        BACKENDS[backend].meta(result)
    except KeyError:
        raise BackendError('No backend {}'.format(backend))
    except TypeError as e:
        # Since we initialize non-implemented backends as None, and None isn't
        # callable then we'll get a TypeError, and we're looking for NoneType
        # in the message. If we get that we really want a
        # BackendNotImplementedError
        if e.message == "'NoneType' object is not callable":
            raise BackendNotImplementedError(
                'meta function for {} not implemented.'.format(backend))
        else:
            # Otherwise re-raise the error
            raise
