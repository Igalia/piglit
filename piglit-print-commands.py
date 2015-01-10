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


from __future__ import print_function
import argparse
import sys
import os
import os.path as path

sys.path.append(path.dirname(path.realpath(sys.argv[0])))
import framework.core as core
import framework.profile
from framework.test import Test, GleanTest


def main():
    core.get_config()
    parser = argparse.ArgumentParser(sys.argv)
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
    args = parser.parse_args()

    opts = core.Options(exclude_filter=args.exclude_tests,
                        include_filter=args.include_tests)

    # Change to the piglit's path
    piglit_dir = path.dirname(path.realpath(sys.argv[0]))
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

    profile._prepare_test_list(opts)
    for name, test in profile.test_list.items():
        assert(isinstance(test, Test))
        print(name, ':::', getCommand(test))


if __name__ == "__main__":
    main()
