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

""" Module implementing tests for test/all.py and friends

This module only provides tests for native piiglit tests (OpenGL and OpenCL),
it does not provide tests for non native tests that use piglit (oglconfrom,
es3conform, etc)

"""

from __future__ import print_function, absolute_import
import importlib
import os.path as path

from nose.plugins.skip import SkipTest

import framework.tests.utils as utils


@utils.nose_generator
def gen_test_import():
    """ Generates a bunch of tests to import the various test modules """
    def check_import(module):
        """ Test that a module can be imported """
        if not path.exists('bin'):
            raise SkipTest(
                "Piglit has not been compiled, this test will not work")

        importlib.import_module(module)

    # Test the various OpenGL modules
    for module in ['all', 'quick', 'gpu', 'sanity', 'cpu', 'llvmpipe', 'cl',
                   'quick_cl']:
        check_import.description = "Test import of tests.{}".format(module)
        yield check_import, "tests." + module
