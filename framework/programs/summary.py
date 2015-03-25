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

from __future__ import print_function, absolute_import
import argparse
import shutil
import os
import os.path as path
import sys
import errno

from framework import summary, status, core, backends

__all__ = [
    'aggregate',
    'console',
    'csv',
    'html',
]


def html(input_):
    # Make a copy of the status text list and add all. This is used as the
    # argument list for -e/--exclude
    statuses = set(str(s) for s in status.ALL)
    statuses.add('all')

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
                        choices=statuses,
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
    args = parser.parse_args(input_)

    # If args.list and args.resultsFiles are empty, then raise an error
    if not args.list and not args.resultsFiles:
        raise parser.error("Missing required option -l or <resultsFiles>")

    # Convert the exclude_details list to status objects, without this using
    # the -e option will except
    if args.exclude_details:
        # If exclude-results has all, then change it to be all
        if 'all' in args.exclude_details:
            args.exclude_details = status.ALL
        else:
            args.exclude_details = frozenset(
                status.status_lookup(i) for i in args.exclude_details)


    # if overwrite is requested delete the output directory
    if path.exists(args.summaryDir) and args.overwrite:
        shutil.rmtree(args.summaryDir)

    # If the requested directory doesn't exist, create it or throw an error
    core.checkDir(args.summaryDir, not args.overwrite)

    # Merge args.list and args.resultsFiles
    if args.list:
        args.resultsFiles.extend(core.parse_listfile(args.list))

    # Create the HTML output
    output = summary.Summary(args.resultsFiles)
    output.generate_html(args.summaryDir, args.exclude_details)


def console(input_):
    parser = argparse.ArgumentParser()

    # Set the -d and -s options as exclusive, since it's silly to call for diff
    # and then call for only summary
    excGroup1 = parser.add_mutually_exclusive_group()
    excGroup1.add_argument("-d", "--diff",
                           action="store_const",
                           const="diff",
                           dest='mode',
                           help="Only display the differences between multiple "
                                "result files")
    excGroup1.add_argument("-s", "--summary",
                           action="store_const",
                           const="summary",
                           dest='mode',
                           help="Only display the summary, not the individual "
                                "test results")
    excGroup1.add_argument("-i", "--incomplete",
                           action="store_const",
                           const="incomplete",
                           dest='mode',
                           help="Only display tests that are incomplete.")
    parser.add_argument("-l", "--list",
                        action="store",
                        help="Use test results from a list file")
    parser.add_argument("results",
                        metavar="<Results Path(s)>",
                        nargs="+",
                        help="Space seperated paths to at least one results "
                             "file")
    args = parser.parse_args(input_)

    # Throw an error if -d/--diff is called, but only one results file is
    # provided
    if args.mode == 'diff' and len(args.results) < 2:
        parser.error('-d/--diff cannot be specified unless two or more '
                     'results files are specified')

    # make list of results
    if args.list:
        args.results.extend(core.parse_listfile(args.list))

    # Generate the output
    output = summary.Summary(args.results)
    output.generate_text(args.mode)


def csv(input_):
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output",
                        metavar="<Output File>",
                        action="store",
                        dest="output",
                        default="stdout",
                        help="Output filename")
    parser.add_argument("testResults",
                        metavar="<Input Files>",
                        help="JSON results file to be converted")
    args = parser.parse_args(input_)

    try:
        testrun = backends.load(args.testResults)
    except backends.errors.ResultsLoadError as e:
        print('Error: {}'.format(e.message), file=sys.stderr)
        sys.exit(1)

    def write_results(output):
        for name, result in testrun.tests.iteritems():
            output.write("{},{},{},{}\n".format(name, result['time'],
                                                result['returncode'],
                                                result['result']))

    if args.output != "stdout":
        with open(args.output, 'w') as output:
            write_results(output)
    else:
        write_results(sys.stdout)


def aggregate(input_):
    """Combine files in a tests/ directory into a single results file."""
    parser = argparse.ArgumentParser()
    parser.add_argument('results_folder',
                        type=path.realpath,
                        metavar="<results path>",
                        help="Path to a results folder")
    parser.add_argument('-o', '--output',
                        default="results.json",
                        help="name of output file. Default: results.json")
    args = parser.parse_args(input_)

    assert os.path.isdir(args.results_folder)

    outfile = os.path.join(args.results_folder, args.output)
    try:
        results = backends.load(args.results_folder)
    except backends.errors.ResultsLoadError as e:
        print('Error: {}'.format(e.message), file=sys.stderr)
        sys.exit(1)

    try:
        # FIXME: This works, it fixes the problem, but it only works because
        # only the json backend has the ability to agregate results at the
        # moment.
        backends.json._write(results, outfile)
    except IOError as e:
        if e.errno == errno.EPERM:
            print("Error: Unable to write aggregated file, permission denied.",
                  file=sys.stderr)
            sys.exit(1)
        raise

    print("Aggregated file written to: {}".format(outfile))
