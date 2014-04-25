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

""" Provides tests for external integration with piglit

These tests are by no means comprehensive, nor are they meant to be. The goal
is mainly just a sanity check to make sure the modules don't contain syntax
errors and to ensure that the API hasn't changed without fixing these modules

"""

import importlib
from nose.plugins.skip import SkipTest


def _import(name):
    """ Helper for importing modules """
    try:
        return importlib.import_module(name)
    except SystemExit:
        raise SkipTest('Missing symlink, unable to test')


def test_xts_import():
    """ xts.py can be imported """
    _import('tests.xts')


def test_xts_xtstest():
    """ xts.XTSTest initializes """
    mod = _import('tests.xts')
    mod.XTSTest('name', 'testname', 'testnum')


def test_xts_xtsprofile():
    """ xts.XTSProfile initializes """
    mod = _import('tests.xts')
    mod.XTSProfile()
