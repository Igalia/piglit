#!/usr/bin/env python

# Copyright (c) 2015-2016 Intel Corporation

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

import nose.tools as nt

from .. import utils


GENERATOR_DIR = os.path.abspath(
    os.path.join(os.path.dirname(__file__), '..', '..', 'generated_tests'))

BLACKLIST = {
    # These will never need to be tested, they're modules.
    'builtin_function.py',
    'builtin_function_fp64.py',
    'genclbuiltins.py',

    # these (or some subset) should run eventually.
    'random_ubo.py',
    'random_ubo_trim.py',
    'random_ubo-arb_uniform_buffer_object.py',
}

BLACKLIST = set(os.path.join(GENERATOR_DIR, _p) for _p in BLACKLIST)


def discover_generators():
    """Discover all of the generators and return that as a set.

    Removes all generators in the BLACKLIST constant.

    """
    def fqp(path):
        """make fully-qualified path."""
        return os.path.abspath(os.path.join(GENERATOR_DIR, path))

    contents = set(fqp(p) for p in os.listdir(GENERATOR_DIR)
                   if p.endswith('.py'))
    contents.difference_update(BLACKLIST)
    return contents


@utils.nose_generator
def test_generators():
    """Generate tests for the various generators."""

    @utils.test_in_tempdir
    def test(name):
        """Tester function."""
        msg = ''

        with open(os.devnull, 'w') as d:
            proc = subprocess.Popen(['python', name],
                                    stderr=subprocess.PIPE,
                                    stdout=d)
            _, err = proc.communicate()

        if proc.returncode != 0:
            err = err.decode('utf-8')

            raise utils.TestFailure(
                "failed with message:\n {}".format(err))

    description = 'generator: {} runs successfully'

    for generator in discover_generators():
        test.description = description.format(os.path.basename(generator))
        yield test, generator
