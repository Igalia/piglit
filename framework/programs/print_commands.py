# coding=utf-8
# Copyright (c) 2016 Intel Corporation

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

"""Print each test's command in a user-defined consumable format.

This is meant to provide a mechanism to export piglit commands to other tools,
or to create lists of tests to be consumed via "piglit run --test-list"

"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import argparse
import os
import sys

import six

from . import parsers
from framework import options, profile, exceptions
from framework.test import Test


def get_command(test, piglit_dir):
    """Get just the name of the command with a path relative to bin."""

    # Make the test command relative to the piglit_dir
    test_command = test.command[:]
    test_command[0] = os.path.relpath(test_command[0], piglit_dir)

    command = ' '.join(test_command)
    return command


@exceptions.handler
def main(input_):
    """The main function."""
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
    parser.add_argument("--format",
                        dest="format_string",
                        default="{name} ::: {command}",
                        action="store",
                        help="A template string that defines the output "
                             "format. It has two replacement tokens that can "
                             "be provided, along with any arbitrary text, "
                             "which will be printed verbatim. The two tokens "
                             "are '{name}', which will be replaced with the "
                             "name of the test; and '{command}', which will "
                             "be replaced with the command to run the test.")
    parser.add_argument("testProfile",
                        metavar="<Path to testfile>",
                        help="Path to results folder")
    args = parser.parse_args(input_)

    profile_ = profile.load_test_profile(args.testProfile)

    if args.exclude_tests:
        profile_.filters.append(profile.RegexFilter(args.exclude_tests))
    if args.include_tests:
        profile_.filters.append(profile.RegexFilter(args.include_tests))

    # Change to the piglit's path
    piglit_dir = os.path.dirname(os.path.realpath(sys.argv[0]))
    os.chdir(piglit_dir)

    for name, test in profile_.itertests():
        assert isinstance(test, Test)
        print(args.format_string.format(
            name=name,
            command=get_command(test, piglit_dir)))
