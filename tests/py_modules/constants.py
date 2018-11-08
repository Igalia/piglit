# coding=utf-8
# Copyright (c) 2015, 2016 Intel Corporation

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

"""Shared constants for test modules."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os
import sys
import subprocess

TESTS_DIR = os.path.abspath(os.path.join(os.path.dirname(__file__), '..'))

# If the PIGLIT_BUILD_DIR env var is set, we want to look for the
# generated tests in $PIGLIT_BUILD_DIR/generated_tests/.  Otherwise,
# look in TESTS_DIR/../generated_tests.
build_dir = os.environ.get('PIGLIT_BUILD_TREE')
if not build_dir:
    build_dir = os.environ.get('PIGLIT_BUILD_DIR', os.path.join(TESTS_DIR, '..'))

GENERATED_TESTS_DIR = os.path.abspath(os.path.join(build_dir, 'generated_tests'))

# If on cygwin convert to a dos style path
if sys.platform == 'cygwin':
    def dosify(p):
        return subprocess.check_output(
            ['cygpath', '-d', p]).rstrip().decode('utf-8')

    TESTS_DIR = dosify(TESTS_DIR)
    GENERATED_TESTS_DIR = dosify(GENERATED_TESTS_DIR)
