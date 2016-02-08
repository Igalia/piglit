#!/usr/bin/env python

# Copyright (c) 2015 Intel Corporation

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

"""Generate tests for running the generators.

This needs to be compatible with both python2 and python3.

"""

from __future__ import absolute_import, division, print_function
import os
import subprocess
import functools

import nose.tools as nt


BLACKLIST = {
    'builtin_function.py',
    'builtin_function_fp64.py',
    'genclbuiltins.py',
    'test_generators.py',

    # these (or some subset) should run eventually.
    'random_ubo.py',
    'random_ubo_trim.py',
    'random_ubo-arb_uniform_buffer_object.py',
}

BLACKLIST = set([os.path.abspath(os.path.join(os.path.dirname(__file__), _p))
                 for _p in BLACKLIST])


def discover_generators():
    """Discover all of the generators and return that as a set.

    Removes all generators in the BLACKLIST constant.

    """
    def fqp(path):
        """make fully-qualified path."""
        return os.path.abspath(os.path.join(os.path.dirname(__file__), path))

    contents = set([fqp(p) for p in os.listdir(os.path.dirname(__file__))
                    if p.endswith('.py')])
    contents.difference_update(BLACKLIST)
    return contents


class GeneratedTestWrapper(object):  # pylint: disable=too-few-public-methods
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

    @functools.wraps(func)
    def test_wrapper(*args, **kwargs):
        for x in func(*args, **kwargs):
            x = list(x)
            x[0] = GeneratedTestWrapper(x[0])
            yield tuple(x)  # This must be a tuple for some reason

    return test_wrapper


@nose_generator
def test_generators():
    """Generate tests for the various generators."""

    def test(name):
        """Tester function."""
        msg = ''

        try:
            with open(os.devnull, 'w') as d:
                rcode = subprocess.check_call(['python', name], stderr=d,
                                              stdout=d)
        except subprocess.CalledProcessError as e:
            msg = "While calling {}:\n{}".format(name, str(e))
            rcode = e.returncode

        nt.eq_(rcode, 0, msg)

    description = 'generator: {} runs successfully'

    for generator in discover_generators():
        test.description = description.format(os.path.basename(generator))
        yield test, generator
