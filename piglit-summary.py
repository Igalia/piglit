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
import string

sys.path.append(os.path.dirname(os.path.realpath(sys.argv[0])))
import framework.core as core
import framework.summary


def parse_listfile(filename):
    """
    Read a list of newline seperated file names and return them as a list
    """
    return open(filename, "r").read().rstrip().split('\n')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-d", "--diff",
                        action  = "store_true",
                        help    = "Only display the differences between"
                                  "multiple result files")
    parser.add_argument("-s", "--summary",
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

    # make list of results
    if args.list:
        args.results.extend(parse_listfile(args.list))
    results = [core.loadTestResults(i) for i in args.results]

    summary = framework.summary.Summary(results)

    # possible test outcomes
    possible_results = [ "pass", "fail", "crash", "skip", "warn" ]
    if len(args.results) > 1:
        possible_results.append("changes")

    # init the summary counters
    counts = {}
    for result in possible_results:
        counts[result] = 0

    # get all results
    all = summary.allTests()

    # sort the results list by path
    all = sorted(all, key=lambda test: test.path)

    # loop over the tests
    for test in all:
        results = []
        anyChange = False
        # loop over the results for multiple runs
        for j in range(len(summary.testruns)):
            outcome = test.results[j]['result'] # 'pass', 'fail', etc.
            # check for different results between multiple runs
            if len(results) >= 1 and not outcome in results:
                # something changed
                counts["changes"] += 1
                anyChange = True
            results.append(outcome)

        # if all test runs had the same outcome:
        if not anyChange:
            counts[outcome] += 1

        # print the individual test result line
        if args.diff:
            if anyChange:
                print "%s: %s" % (test.path, string.join(results," "))
        elif not args.summary:
            print "%s: %s" % (test.path, string.join(results," "))

    # print the summary info
    print "summary:"
    total = 0
    for result in possible_results:
        print " %7s: %5d" % (result, counts[result])
        total += counts[result]
    print "   total: %5d" % total


if __name__ == "__main__":
    main()
