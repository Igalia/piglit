#!/usr/bin/env python
#
# Copyright (c) 2013 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

import argparse
import unittest

import framework.tests.summary

# Create a dictionary of all of tests. Do this before the parser so we can use
# it as a list of optional arguments for the parser
tests = {"summary": unittest.TestLoader().loadTestsFromModule(framework.tests.summary)}

parser = argparse.ArgumentParser()
parser.add_argument("tests",
                    action="append",
                    choices=tests.keys(),
                    help="Testing profiles for the framework")
parser.add_argument("-v", "--verbose",
                    action="store",
                    choices=['0', '1', '2'],
                    default='1',
                    help="Set the level of verbosity to run tests at")
args = parser.parse_args()

# Run the tests
map(unittest.TextTestRunner(verbosity=int(args.verbose)).run,
    [tests[x] for x in args.tests])
