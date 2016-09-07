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
import os.path as path
import time
import ctypes
import shutil

import six

from framework import core, backends, exceptions, options
import framework.results
import framework.profile
from . import parsers

__all__ = ['run',
           'resume']


def booltype(val):
    if val.lower() in ['false', 'no', '0']:
        return False
    elif val.lower() in ['true', 'yes', '1']:
        return True
    raise argparse.ArgumentTypeError(
        'Case insensitve values of "yes", "no", "false", "true", and "0" or '
        '"1" are accepted.')


def _default_platform():
    """ Logic to determine the default platform to use

    This assumes that the platform can only be set on Linux, it probably works
    on BSD. This is only relevant if piglit is built with waffle support. When
    waffle support lands for Windows and if it ever happens for OSX, this will
    need to be extended.

    On Linux this will try in order,
    1) An option provided via the -p/--platform option (this is handled in
       argparse, not in this function)
    2) PIGLIT_PLATFORM from the environment
    3) [core]:platform from the config file
    4) mixed_glx_egl

    """
    if os.environ.get('PIGLIT_PLATFORM'):
        return os.environ.get('PIGLIT_PLATFORM')
    else:
        plat = core.PIGLIT_CONFIG.safe_get('core', 'platform', 'mixed_glx_egl')
        if plat not in core.PLATFORMS:
            raise exceptions.PiglitFatalError(
                'Platform is not valid\nvalid platforms are: {}'.format(
                    core.PLATFORMS))
        return plat


def _default_backend():
    """ Logic to se the default backend to use

    There are two options, either the one set via the -b/--backend option, or
    the one in the config file. The default if that fails is to use json

    """
    backend = core.PIGLIT_CONFIG.safe_get('core', 'backend', 'json')
    if backend not in backends.BACKENDS.keys():
        raise exceptions.PiglitFatalError(
            'Backend is not valid\nvalid backends are: {}'.format(
                ' '.join(backends.BACKENDS.keys())))
    return backend


def _run_parser(input_):
    """ Parser for piglit run command """
    unparsed = parsers.parse_config(input_)[1]

    # Set the parent of the config to add the -f/--config message
    parser = argparse.ArgumentParser(parents=[parsers.CONFIG])
    parser.add_argument("-n", "--name",
                        metavar="<test name>",
                        default=None,
                        help="Name of this test run")
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
    parser.add_argument('-b', '--backend',
                        default=_default_backend(),
                        choices=backends.BACKENDS.keys(),
                        help='select a results backend to use')
    conc_parser = parser.add_mutually_exclusive_group()
    conc_parser.add_argument('-c', '--all-concurrent',
                             action="store_const",
                             default="some",
                             const="all",
                             dest="concurrency",
                             help="Run all tests concurrently")
    conc_parser.add_argument("-1", "--no-concurrency",
                             action="store_const",
                             default="some",
                             const="none",
                             dest="concurrency",
                             help="Disable concurrent test runs")
    parser.add_argument("-p", "--platform",
                        choices=core.PLATFORMS,
                        default=_default_platform(),
                        help="Name of windows system passed to waffle")
    parser.add_argument("--valgrind",
                        action="store_true",
                        help="Run tests in valgrind's memcheck")
    parser.add_argument("--dmesg",
                        action="store_true",
                        help="Capture a difference in dmesg before and "
                             "after each test. Implies -1/--no-concurrency")
    parser.add_argument("--abort-on-monitored-error",
                        action="store_true",
                        dest="monitored",
                        help="Enable monitoring according the rules defined "
                             "in piglit.conf, and stop the execution when a "
                             "monitored error is detected. Exit code 3. "
                             "Implies -1/--no-concurrency")
    parser.add_argument("-s", "--sync",
                        action="store_true",
                        help="Sync results to disk after every test")
    parser.add_argument("--junit_suffix",
                        type=str,
                        default="",
                        help="suffix string to append to each test name in junit")
    parser.add_argument("--junit-subtests",
                        action='store_true',
                        help="Encode tests with subtets as testsuite elements. "
                             "Some xUnit parsers do not handle nested "
                             "testsuites, though it is allowed in the spec.")
    log_parser = parser.add_mutually_exclusive_group()
    log_parser.add_argument('-v', '--verbose',
                            action='store_const',
                            const='verbose',
                            default='quiet',
                            dest='log_level',
                            help='DEPRECATED! Print more information during '
                                 'test runs. Use -l/--log-level instead')
    log_parser.add_argument("-l", "--log-level",
                            dest="log_level",
                            action="store",
                            choices=['quiet', 'verbose', 'dummy', 'http'],
                            default='quiet',
                            help="Set the logger verbosity level")
    parser.add_argument("--test-list",
                        type=os.path.abspath,
                        help="A file containing a list of tests to run")
    parser.add_argument('-o', '--overwrite',
                        dest='overwrite',
                        action='store_true',
                        help='If the results_path already exists, delete it')
    parser.add_argument('--deqp-mustpass-list',
                        dest='deqp_mustpass',
                        action='store_true',
                        help='Run only the tests in the deqp mustpass list '
                             'when running a deqp gles{2,3,31} profile, '
                             'otherwise run all tests.')
    parser.add_argument('--process-isolation',
                        dest='process_isolation',
                        action='store',
                        type=booltype,
                        default=core.PIGLIT_CONFIG.safe_get(
                            'core', 'process isolation', 'true'),
                        metavar='<bool>',
                        help='Set this to allow tests to run without process '
                             'isolation. This allows, but does not require, '
                             'tests to run multiple tests per process. '
                             'This value can also be set in piglit.conf.')
    parser.add_argument("test_profile",
                        metavar="<Profile path(s)>",
                        nargs='+',
                        help="Path to one or more test profiles to run. "
                             "If more than one profile is provided then they "
                             "will be merged.")
    parser.add_argument("results_path",
                        type=path.realpath,
                        metavar="<Results Path>",
                        help="Path to results folder")
    return parser.parse_args(unparsed)


def _create_metadata(args, name):
    """Create and return a metadata dict for Backend.initialize()."""
    opts = dict(options.OPTIONS)
    opts['profile'] = args.test_profile
    opts['log_level'] = args.log_level
    if args.platform:
        opts['platform'] = args.platform

    metadata = {'options': opts}
    metadata['name'] = name
    metadata.update(core.collect_system_info())

    return metadata


def _disable_windows_exception_messages():
    """Disable Windows error message boxes for this and all child processes."""
    if sys.platform == 'win32':
        # This disables messages boxes for uncaught exceptions, but it will not
        # disable the message boxes for assertion failures or abort().  Those
        # are created not by the system but by the CRT itself, and must be
        # disabled by the child processes themselves.
        SEM_FAILCRITICALERRORS     = 0x0001
        SEM_NOALIGNMENTFAULTEXCEPT = 0x0004
        SEM_NOGPFAULTERRORBOX      = 0x0002
        SEM_NOOPENFILEERRORBOX     = 0x8000
        uMode = ctypes.windll.kernel32.SetErrorMode(0)
        uMode |= SEM_FAILCRITICALERRORS \
              |  SEM_NOALIGNMENTFAULTEXCEPT \
              |  SEM_NOGPFAULTERRORBOX \
              |  SEM_NOOPENFILEERRORBOX
        ctypes.windll.kernel32.SetErrorMode(uMode)


def _results_handler(path):
    """Handler for core.check_dir."""
    if os.path.isdir(path):
        shutil.rmtree(path)
    else:
        os.unlink(path)


@exceptions.handler
def run(input_):
    """ Function for piglit run command

    This is a function because it allows it to be shared between piglit-run.py
    and piglit run

    """
    args = _run_parser(input_)
    _disable_windows_exception_messages()

    # If dmesg is requested we must have serial run, this is because dmesg
    # isn't reliable with threaded run
    if args.dmesg or args.monitored:
        args.concurrency = "none"

    # Pass arguments into Options
    options.OPTIONS.concurrent = args.concurrency
    options.OPTIONS.exclude_filter = args.exclude_tests
    options.OPTIONS.include_filter = args.include_tests
    options.OPTIONS.execute = args.execute
    options.OPTIONS.valgrind = args.valgrind
    options.OPTIONS.dmesg = args.dmesg
    options.OPTIONS.monitored = args.monitored
    options.OPTIONS.sync = args.sync
    options.OPTIONS.deqp_mustpass = args.deqp_mustpass
    options.OPTIONS.process_isolation = args.process_isolation

    # Set the platform to pass to waffle
    options.OPTIONS.env['PIGLIT_PLATFORM'] = args.platform

    # Change working directory to the root of the piglit directory
    piglit_dir = path.dirname(path.realpath(sys.argv[0]))
    os.chdir(piglit_dir)

    # If the results directory already exists and if overwrite was set, then
    # clear the directory. If it wasn't set, then raise fatal error.
    try:
        core.check_dir(args.results_path,
                       failifexists=args.overwrite,
                       handler=_results_handler)
    except exceptions.PiglitException:
        raise exceptions.PiglitFatalError(
            'Cannot overwrite existing folder without the -o/--overwrite '
            'option being set.')

    results = framework.results.TestrunResult()
    backends.set_meta(args.backend, results)

    # Set results.name
    if args.name is not None:
        results.name = args.name
    else:
        results.name = path.basename(args.results_path)

    backend = backends.get_backend(args.backend)(
        args.results_path,
        junit_suffix=args.junit_suffix,
        junit_subtests=args.junit_subtests)
    backend.initialize(_create_metadata(args, results.name))

    profile = framework.profile.merge_test_profiles(args.test_profile)
    profile.results_dir = args.results_path
    # If a test list is provided then set the forced_test_list value.
    if args.test_list:
        with open(args.test_list) as test_list:
            # Strip newlines
            profile.forced_test_list = list([t.strip() for t in test_list])

    results.time_elapsed.start = time.time()
    # Set the dmesg type
    if args.dmesg:
        profile.dmesg = args.dmesg

    if args.monitored:
        profile.monitoring = args.monitored

    profile.run(args.log_level, backend)

    results.time_elapsed.end = time.time()
    backend.finalize({'time_elapsed': results.time_elapsed})

    print('Thank you for running Piglit!\n'
          'Results have been written to ' + args.results_path)


@exceptions.handler
def resume(input_):
    parser = argparse.ArgumentParser()
    parser.add_argument("results_path",
                        type=path.realpath,
                        metavar="<Results Path>",
                        help="Path to results folder")
    parser.add_argument("-f", "--config",
                        dest="config_file",
                        type=argparse.FileType("r"),
                        help="Optionally specify a piglit config file to use. "
                             "Default is piglit.conf")
    parser.add_argument("-n", "--no-retry",
                        dest="no_retry",
                        action="store_true",
                        help="Do not retry incomplete tests")
    args = parser.parse_args(input_)
    _disable_windows_exception_messages()

    results = backends.load(args.results_path)
    options.OPTIONS.concurrent = results.options['concurrent']
    options.OPTIONS.exclude_filter = results.options['exclude_filter']
    options.OPTIONS.include_filter = results.options['include_filter']
    options.OPTIONS.execute = results.options['execute']
    options.OPTIONS.valgrind = results.options['valgrind']
    options.OPTIONS.dmesg = results.options['dmesg']
    options.OPTIONS.monitored = results.options['monitored']
    options.OPTIONS.sync = results.options['sync']
    options.OPTIONS.deqp_mustpass = results.options['deqp_mustpass']
    options.OPTIONS.proces_isolation = results.options['process_isolation']

    core.get_config(args.config_file)

    options.OPTIONS.env['PIGLIT_PLATFORM'] = results.options['platform']

    results.options['env'] = core.collect_system_info()
    results.options['name'] = results.name

    # Resume only works with the JSON backend
    backend = backends.get_backend('json')(
        args.results_path,
        file_start_count=len(results.tests) + 1)
    # Specifically do not initialize again, everything initialize does is done.

    # Don't re-run tests that have already completed, incomplete status tests
    # have obviously not completed.
    for name, result in six.iteritems(results.tests):
        if args.no_retry or result.result != 'incomplete':
            options.OPTIONS.exclude_tests.add(name)

    profile = framework.profile.merge_test_profiles(results.options['profile'])
    profile.results_dir = args.results_path
    if options.OPTIONS.dmesg:
        profile.dmesg = options.OPTIONS.dmesg

    if options.OPTIONS.monitored:
        profile.monitoring = options.OPTIONS.monitored

    # This is resumed, don't bother with time since it won't be accurate anyway
    profile.run(results.options['log_level'], backend)

    backend.finalize()

    print("Thank you for running Piglit!\n"
          "Results have been written to {0}".format(args.results_path))
