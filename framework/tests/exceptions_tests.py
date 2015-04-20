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

"""Tests for the exceptions module."""

from __future__ import print_function, absolute_import, division

import nose.tools as nt

from framework.tests import utils
from framework import exceptions


@nt.raises(SystemExit)
@utils.capture_stderr
@exceptions.handler
def test_handle_PiglitFatalError():
    """exceptions.handler: Handles PiglitFatalError"""
    raise exceptions.PiglitFatalError


@nt.raises(SystemExit)
@utils.capture_stderr
@exceptions.handler
def test_handle_PiglitInternalError():
    """exceptions.handler: Handles PiglitInternalError"""
    raise exceptions.PiglitInternalError


@nt.raises(SystemExit)
@utils.capture_stderr
@exceptions.handler
def test_handle_PiglitException():
    """exceptions.handler: Handles PiglitException"""
    raise exceptions.PiglitException


@nt.raises(SystemExit)
@utils.capture_stderr
@exceptions.handler
def test_handle_Exception():
    """exceptions.handler: Handles Exception"""
    raise Exception
