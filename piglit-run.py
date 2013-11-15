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
import os
import os.path as path
import time
import traceback

sys.path.append(path.dirname(path.realpath(sys.argv[0])))
import framework.core as core
from framework.threads import synchronized_self


def main():
    parser = argparse.ArgumentParser(sys.argv)
    # Either require that a name for the test is passed or that
    # resume is requested
    excGroup1 = parser.add_mutually_exclusive_group()
    excGroup1.add_argument("-n", "--name",
                           metavar="<test name>",
                           default=None,
                           help="Name of this test run")
    excGroup1.add_argument("-r", "--resume",
                           action="store_true",
                           help="Resume an interupted test run")
    # Setting the --dry-run flag is equivalent to env.execute=false
    parser.add_argument("-d", "--dry-run",
                        action="store_false",
                        dest="execute",
                        help="Do not execute the tests")
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
                        help="Exclude matching tests "
                             "(can be used more than once)")
    parser.add_argument("-1", "--no-concurrency",
                        action="store_false",
                        dest="concurrency",
                        help="Disable concurrent test runs")
    parser.add_argument("-p", "--platform",
                        choices=["glx", "x11_egl", "wayland", "gbm"],
                        help="Name of windows system passed to waffle")
    parser.add_argument("--valgrind",
                        action="store_true",
                        help="Run tests in valgrind's memcheck")
    parser.add_argument("--dmesg",
                        action="store_true",
                        help="Capture a difference in dmesg before and "
                             "after each test")
    parser.add_argument("test_profile",
                        metavar="<Path to test profile>",
                        help="Path to testfile to run")
    parser.add_argument("results_path",
                        type=path.realpath,
                        metavar="<Results Path>",
                        help="Path to results folder")
    args = parser.parse_args()

    # Set the platform to pass to waffle
    if args.platform:
        os.environ['PIGLIT_PLATFORM'] = args.platform

    # If resume is requested attempt to load the results file
    # in the specified path
    if args.resume is True:
        # Load settings from the old results JSON
        old_results = core.load_results(args.results_path)
        args.test_profile = old_results.options['profile']

        # Changing the args to the old args allows us to set them
        # all in one places down the way
        args.exclude_tests = old_results.options['exclude_filter']
        args.include_tests = old_results.options['filter']
        args.concurrency = old_results.options['concurrency']

    # Otherwise parse additional settings from the command line

    # Pass arguments into Environment
    env = core.Environment(concurrent=args.concurrency,
                           exclude_filter=args.exclude_tests,
                           include_filter=args.include_tests,
                           execute=args.execute,
                           valgrind=args.valgrind,
                           dmesg=args.dmesg)

    # Change working directory to the root of the piglit directory
    piglit_dir = path.dirname(path.realpath(sys.argv[0]))
    os.chdir(piglit_dir)

    core.checkDir(args.results_path, False)

    results = core.TestrunResult()

    # Set results.name
    if args.name is not None:
        results.name = args.name
    else:
        results.name = path.basename(args.results_path)

    # Begin json.
    result_filepath = path.join(args.results_path, 'main')
    result_file = open(result_filepath, 'w')
    json_writer = core.JSONWriter(result_file)
    json_writer.open_dict()

    # Write out command line options for use in resuming.
    json_writer.write_dict_key('options')
    json_writer.open_dict()
    json_writer.write_dict_item('profile', args.test_profile)
    json_writer.write_dict_item('filter', args.include_tests)
    json_writer.write_dict_item('exclude_filter', args.exclude_tests)
    json_writer.write_dict_item('concurrency', args.concurrency)
    json_writer.close_dict()

    json_writer.write_dict_item('name', results.name)
    for (key, value) in env.collectData().items():
        json_writer.write_dict_item(key, value)

    profile = core.loadTestProfile(args.test_profile)

    json_writer.write_dict_key('tests')
    json_writer.open_dict()
    # If resuming an interrupted test run, re-write all of the existing
    # results since we clobbered the results file.  Also, exclude them
    # from being run again.
    if args.resume is True:
        for (key, value) in old_results.tests.items():
            if os.path.sep != '/':
                key = key.replace(os.path.sep, '/', -1)
            json_writer.write_dict_item(key, value)
            env.exclude_tests.add(key)

    time_start = time.time()
    profile.run(env, json_writer)
    time_end = time.time()

    json_writer.close_dict()

    results.time_elapsed = time_end - time_start
    json_writer.write_dict_item('time_elapsed', results.time_elapsed)

    # End json.
    json_writer.close_dict()
    json_writer.file.close()

    print
    print 'Thank you for running Piglit!'
    print 'Results have been written to ' + result_filepath


if __name__ == "__main__":
    main()
