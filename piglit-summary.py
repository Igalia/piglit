#!/usr/bin/env python
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

# Print a very simple summary of piglit results file(s).
# When multiple result files are specified, compare the results
# of each test run to look for differences/regressions.
#
# Brian Paul
# April 2013


import argparse
import os.path
import sys

sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
import framework.summary as summary


def parse_listfile(filename):
    """
    Read a list of newline seperated file names and return them as a list
    """
    return open(filename, "r").read().rstrip().split('\n')



def main():
    parser = argparse.ArgumentParser()

    # Set the -d and -s options as exclusive, since it's silly to call for diff
    # and then call for only summary
    excGroup1 = parser.add_mutually_exclusive_group()
    excGroup1.add_argument("-d", "--diff",
                        action  = "store_true",
                        help    = "Only display the differences between"
                                  "multiple result files")
    excGroup1.add_argument("-s", "--summary",
                        action  = "store_true",
                        help    = "Only display the summary, not the"
                                  "individual test results")

    parser.add_argument("-l", "--list",
                        action  = "store",
                        help    = "Use test results from a list file")
    parser.add_argument("results",
                        metavar = "<Results Path(s)>",
                        nargs   = "+",
                        help    = "Space seperated paths to at least one"
                                  "results file")
    args = parser.parse_args()

    # Throw an error if -d/--diff is called, but only one results file is
    # provided
    if args.diff and len(args.results) < 2:
        parser.error('-d/--diff cannot be specified unless two or more '
                     'results files are specified')

    # make list of results
    if args.list:
        args.results.extend(parse_listfile(args.list))

    # Generate the output
    output = summary.Summary(args.results)
    output.generateText(args.diff, args.summary)


if __name__ == "__main__":
    main()
