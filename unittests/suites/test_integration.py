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

"""Provides tests for external integration with piglit.

These tests are by no means comprehensive, nor are they meant to be. The goal
is mainly just a sanity check to make sure the modules don't contain syntax
errors and to ensure that the API hasn't changed without fixing these modules
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import importlib

import pytest

from framework import core
from framework import exceptions


def setup_module():
    core.PIGLIT_CONFIG = core.PiglitConfig(allow_no_value=True)
    core.get_config()


def _import(name):
    """Helper for importing modules.

    It is very important that we use import_module to get the module, since we
    need more than just the profile, since we want to ensure that the Test
    derived class is importable.
    """
    try:
        return importlib.import_module(name)
    except exceptions.PiglitFatalError:
        pytest.skip('The module experienced a fatal error. '
                    'This may be expected.')


@pytest.mark.parametrize("name", [
    'tests.cts_gl',
    'tests.cts_gles',
    'tests.deqp_gles2',
    'tests.deqp_gles3',
    'tests.deqp_gles31',
    'tests.deqp_vk',
    'tests.deqp_egl',
    'tests.es3conform',
    'tests.igt',
    'tests.oglconform',
    'tests.xts',
    'tests.xts-render',
])
def test_import(name):
    """Try to import tests, if they raise an error skip."""
    # TODO: Figure out what is necissary for each test to run, or catch
    # specific errors as skips rather than any of them.
    _import(name)


def test_xts_xtstest():
    """ xts.XTSTest initializes """
    mod = _import('tests.xts')
    mod.XTSTest('name', 'testname', 'testnum')


def test_xts_xtsprofile():
    """ xts.XTSProfile initializes """
    mod = _import('tests.xts')
    mod.XTSProfile()


def test_igt_igttest():
    """ igt.IGTTest initializes """
    mod = _import('tests.igt')
    mod.IGTTest('foo')


def test_es3conform_gtftest():
    """ es3conform.GTFTest initializes """
    mod = _import('tests.es3conform')
    mod.GTFTest('testpath')


def test_oglconform_oglctest():
    """ oglconform.OGLCTest initializes """
    mod = _import('tests.oglconform')
    mod.OGLCTest('catagory', 'subtest')
