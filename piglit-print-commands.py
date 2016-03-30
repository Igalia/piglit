#!/usr/bin/env python2
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or
# sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
#
# This permission notice shall be included in all copies or
# substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
# KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
# PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHOR(S) BE
# LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
# AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
# OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

"""Print each test's command in a consumable format."""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import argparse
import os
import sys

sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
from framework import options, profile
from framework.programs import parsers
from framework.test import Test, GleanTest


def get_command(test, piglit_dir):
    """Get just the name of the command with a path relative to bin."""
    command = ''
    if isinstance(test, GleanTest):
        for var, val in test.env.items():
            command += "{}='{}'".format(var, val)

    # Make the test command relative to the piglit_dir
    test_command = test.command[:]
    test_command[0] = os.path.relpath(test_command[0], piglit_dir)

    command += ' '.join(test_command)
    return command


def main():
    """The main function."""
    input_ = [i.decode('utf-8') for i in sys.argv[1:]]
    parser = argparse.ArgumentParser(parents=[parsers.CONFIG])
    parser.add_argument("-t", "--include-tests",
                        default=[],
                        action="append",
                        metavar="<regex>",
                        help="Run only matching tests "
                             "(can be used more than once)")
    parser.add_argument("-x", "--exclude-tests",
                        default=[],
                        action="append",
                        metavar="<regex>",
                        help="Exclude matching tests (can be used more than "
                             "once)")
    parser.add_argument("testProfile",
                        metavar="<Path to testfile>",
                        help="Path to results folder")
    args = parser.parse_args(input_)

    options.OPTIONS.exclude_filter = args.exclude_tests
    options.OPTIONS.include_filter = args.include_tests

    # Change to the piglit's path
    piglit_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
    os.chdir(piglit_dir)

    profile_ = profile.load_test_profile(args.testProfile)

    profile_._prepare_test_list()
    for name, test in profile_.test_list.items():
        assert isinstance(test, Test)
        print(name, ':::', get_command(test, piglit_dir))


if __name__ == "__main__":
    main()
