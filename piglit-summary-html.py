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

import argparse
import sys
import shutil
import os.path as path

import framework.summary as summary
from framework.core import checkDir

sys.path.append(path.dirname(path.realpath(sys.argv[0])))


def parse_listfile(filename):
    """
    Read a list of newline seperated flies and return them as a python list.
    strip the last newline character so the list doesn't have an extra ''
    element at the end.
    """
    with open(filename, 'r') as file:
        return [path.expanduser(i) for i in file.read().rstrip().split('\n')]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--overwrite",
                        action  = "store_true",
                        help    = "Overwrite existing directories")
    parser.add_argument("-l", "--list",
                        action  = "store",
                        help    = "Load a newline seperated list of results. "
                                  "These results will be prepended to any "
                                  "Results specified on the command line")
    parser.add_argument("summaryDir",
                        metavar = "<Summary Directory>",
                        help    = "Directory to put HTML files in")
    parser.add_argument("resultsFiles",
                        metavar = "<Results Files>",
                        nargs   = "*",
                        help    = "Results files to include in HTML")
    args = parser.parse_args()

    # If args.list and args.resultsFiles are empty, then raise an error
    if not args.list and not args.resultsFiles:
        raise parser.error("Missing required option -l or <resultsFiles>")

    # if overwrite is requested delete the output directory
    if path.exists(args.summaryDir) and args.overwrite:
        shutil.rmtree(args.summaryDir)

    # If the requested directory doesn't exist, create it or throw an error
    checkDir(args.summaryDir, not args.overwrite)

    # Merge args.list and args.resultsFiles
    if args.list:
        args.resultsFiles.extend(parse_listfile(args.list))

    # Create the HTML output
    output = summary.NewSummary(args.resultsFiles)
    output.generateHTML(args.summaryDir)


if __name__ == "__main__":
    main()
