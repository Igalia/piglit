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
import framework.status as status
from framework.core import checkDir, parse_listfile

sys.path.append(path.dirname(path.realpath(sys.argv[0])))


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--overwrite",
                        action="store_true",
                        help="Overwrite existing directories")
    parser.add_argument("-l", "--list",
                        action="store",
                        help="Load a newline seperated list of results. These "
                             "results will be prepended to any Results "
                             "specified on the command line")
    parser.add_argument("-e", "--exclude-details",
                        default=[],
                        action="append",
                        choices=['skip', 'pass', 'warn', 'crash' 'fail',
                                 'all'],
                        help="Optionally exclude the generation of HTML pages "
                             "for individual test pages with the status(es) "
                             "given as arguments. This speeds up HTML "
                             "generation, but reduces the info in the HTML "
                             "pages. May be used multiple times")
    parser.add_argument("summaryDir",
                        metavar="<Summary Directory>",
                        help="Directory to put HTML files in")
    parser.add_argument("resultsFiles",
                        metavar="<Results Files>",
                        nargs="*",
                        help="Results files to include in HTML")
    args = parser.parse_args()

    # If args.list and args.resultsFiles are empty, then raise an error
    if not args.list and not args.resultsFiles:
        raise parser.error("Missing required option -l or <resultsFiles>")

    # Convert the exclude_details list to status objects, without this using
    # the -e option will except
    if args.exclude_details:
        # If exclude-results has all, then change it to be all
        if 'all' in args.exclude_details:
            args.exclude_details = [status.Skip(), status.Pass(), status.Warn(),
                                    status.Crash(), status.Fail()]
        else:
            args.exclude_details = [status.status_lookup(i) for i in
                                    args.exclude_details]


    # if overwrite is requested delete the output directory
    if path.exists(args.summaryDir) and args.overwrite:
        shutil.rmtree(args.summaryDir)

    # If the requested directory doesn't exist, create it or throw an error
    checkDir(args.summaryDir, not args.overwrite)

    # Merge args.list and args.resultsFiles
    if args.list:
        args.resultsFiles.extend(parse_listfile(args.list))

    # Create the HTML output
    output = summary.Summary(args.resultsFiles)
    output.generateHTML(args.summaryDir, args.exclude_details)


if __name__ == "__main__":
    main()
