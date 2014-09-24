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
import shutil
import os.path as path
import sys

import framework.summary as summary
import framework.status as status
import framework.core as core
import framework.results
import framework.junit 

__all__ = ['html',
           'junit',
           'console',
           'csv']


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


class _Writer:

    def __init__(self, filename):
        self.report = framework.junit.Report(filename)
        self.path = []

    def write(self, arg):
        testrun = framework.results.load_results(arg)

        self.report.start()
        self.report.startSuite('piglit')
        try:
            for name, result in testrun.tests.iteritems():
                self.write_test(testrun, name, result)
        finally:
            self.enter_path([])
            self.report.stopSuite()
            self.report.stop()

    def write_test(self, testrun, test_path, result):
        test_path = test_path.replace('\\', '/').split('/')
        test_name = test_path.pop()
        self.enter_path(test_path)

        self.report.startCase(test_name)
        duration = None
        try:
            try:
                command = result['command']
            except KeyError:
                pass
            else:
                self.report.addStdout(command + '\n')

            try:
                stdout = result['out']
            except KeyError:
                pass
            else:
                if stdout:
                    self.report.addStdout(stdout + '\n')

            try:
                stderr = result['err']
            except KeyError:
                pass
            else:
                if stderr:
                    self.report.addStderr(stderr + '\n')

            try:
                returncode = result['returncode']
            except KeyError:
                pass
            else:
                if returncode:
                    self.report.addStderr('returncode = %s\n' % returncode)

            success = result.get('result')
            if success in (status.PASS, status.WARN):
                pass
            elif success == status.SKIP:
                self.report.addSkipped()
            elif success == status.CRASH:
                self.report.addError(success.name)
            else:
                self.report.addFailure(success.name)

            try:
                duration = float(result['time'])
            except KeyError:
                pass
        finally:
            self.report.stopCase(duration)

    def enter_path(self, path):
        ancestor = 0
        try:
            while self.path[ancestor] == path[ancestor]:
                ancestor += 1
        except IndexError:
            pass

        for dirname in self.path[ancestor:]:
            self.report.stopSuite()

        for dirname in path[ancestor:]:
            self.report.startSuite(dirname)

        self.path = path


def junit(input_):
    parser = argparse.ArgumentParser()
    parser.add_argument("-o", "--output",
                        metavar="<Output File>",
                        action="store",
                        dest="output",
                        default="piglit.xml",
                        help="Output filename")
    parser.add_argument("testResults",
                        metavar="<Input Files>",
                        help="JSON results file to be converted")
    args = parser.parse_args(input_)

    writer = _Writer(args.output)
    writer.write(args.testResults)


def console(input_):
    parser = argparse.ArgumentParser()

    # Set the -d and -s options as exclusive, since it's silly to call for diff
    # and then call for only summary
    excGroup1 = parser.add_mutually_exclusive_group()
    excGroup1.add_argument("-d", "--diff",
                           action="store_true",
                           help="Only display the differences between multiple "
                                "result files")
    excGroup1.add_argument("-s", "--summary",
                           action="store_true",
                           help="Only display the summary, not the individual "
                                "test results")
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
    if args.diff and len(args.results) < 2:
        parser.error('-d/--diff cannot be specified unless two or more '
                     'results files are specified')

    # make list of results
    if args.list:
        args.results.extend(core.parse_listfile(args.list))

    # Generate the output
    output = summary.Summary(args.results)
    output.generate_text(args.diff, args.summary)


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

    testrun = framework.results.load_results(args.testResults)

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
