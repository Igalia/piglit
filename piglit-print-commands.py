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


from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import argparse
import sys
import os

sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
from framework import options
from framework.programs import parsers
import framework.profile
from framework.test import Test, GleanTest


def main():
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

    profile = framework.profile.load_test_profile(args.testProfile)

    def getCommand(test):
        command = ''
        if isinstance(test, GleanTest):
            for var, val in test.env.items():
                command += var + "='" + val + "' "

        # Make the test command relative to the piglit_dir
        testCommand = test.command[:]
        testCommand[0] = os.path.relpath(testCommand[0], piglit_dir)

        command += ' '.join(testCommand)
        return command

    profile._prepare_test_list()
    for name, test in profile.test_list.items():
        assert(isinstance(test, Test))
        print(name, ':::', getCommand(test))


if __name__ == "__main__":
    main()
