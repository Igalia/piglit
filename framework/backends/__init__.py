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
