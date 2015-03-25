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
import time
import ConfigParser
import ctypes

import framework.core as core
import framework.results
import framework.profile
import framework.backends as backends

__all__ = ['run',
           'resume']


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
        try:
            plat = core.PIGLIT_CONFIG.get('core', 'platform')
            if plat not in core.PLATFORMS:
                print('Platform is not valid\n'
                      'valid platforms are: {}'.format(core.PLATFORMS),
                      file=sys.stderr)
                sys.exit(1)
            return plat
        except (ConfigParser.NoOptionError, ConfigParser.NoSectionError):
            return 'mixed_glx_egl'


def _default_backend():
    """ Logic to se the default backend to use

    There are two options, either the one set via the -b/--backend option, or
    the one in the config file. The default if that fails is to use json

    """
    try:
        backend = core.PIGLIT_CONFIG.get('core', 'backend')
        if backend not in backends.BACKENDS.keys():
            print('Backend is not valid\n',
                  'valid backends are: {}'.format(
                      ' '.join(backends.BACKENDS.keys())),
                  file=sys.stderr)
            sys.exit(1)
        return backend
    except (ConfigParser.NoOptionError, ConfigParser.NoSectionError):
        return 'json'


def _run_parser(input_):
    """ Parser for piglit run command """
    # Parse the config file before any other options, this allows the config
    # file to be used to sete default values for the parser.
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("-f", "--config",
                        dest="config_file",
                        type=argparse.FileType("r"),
                        help="override piglit.conf search path")
    known, unparsed = parser.parse_known_args(input_)

    # Read the config file
    # We want to read this before we finish parsing since some default options
    # are set in the config file.
    core.get_config(known.config_file)

    parser = argparse.ArgumentParser()
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
    parser.add_argument("-s", "--sync",
                        action="store_true",
                        help="Sync results to disk after every test")
    parser.add_argument("--junit_suffix",
                        type=str,
                        default="",
                        help="suffix string to append to each test name in junit")
    # -f/--config is a duplicate that cannot be hit, but is needed for help
    # generation
    parser.add_argument("-f", "--config",
                        metavar="config_file",
                        dest="__unused",
                        help="override piglit.conf search path")
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
                            choices=['quiet', 'verbose', 'dummy'],
                            default='quiet',
                            help="Set the logger verbosity level")
    parser.add_argument("--test-list",
                        help="A file containing a list of tests to run")
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


def _create_metadata(args, name, opts):
    """Create and return a metadata dict for Backend.initialize()."""
    options = {}
    options['profile'] = args.test_profile
    options['log_level'] = args.log_level
    for key, value in opts:
        options[key] = value
    if args.platform:
        options['platform'] = args.platform

    metadata = {'options': options}
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


def run(input_):
    """ Function for piglit run command

    This is a function because it allows it to be shared between piglit-run.py
    and piglit run

    """
    args = _run_parser(input_)
    _disable_windows_exception_messages()

    # If dmesg is requested we must have serial run, this is becasue dmesg
    # isn't reliable with threaded run
    if args.dmesg:
        args.concurrency = "none"

    # build up the include filter based on test_list
    if args.test_list:
        with open(args.test_list) as test_list:
            for line in test_list.readlines():
                args.include_tests.append(line.rstrip())

    # Pass arguments into Options
    opts = core.Options(concurrent=args.concurrency,
                        exclude_filter=args.exclude_tests,
                        include_filter=args.include_tests,
                        execute=args.execute,
                        valgrind=args.valgrind,
                        dmesg=args.dmesg,
                        sync=args.sync)

    # Set the platform to pass to waffle
    opts.env['PIGLIT_PLATFORM'] = args.platform

    # Change working directory to the root of the piglit directory
    piglit_dir = path.dirname(path.realpath(sys.argv[0]))
    os.chdir(piglit_dir)
    core.checkDir(args.results_path, False)

    results = framework.results.TestrunResult()
    backends.set_meta(args.backend, results)

    # Set results.name
    if args.name is not None:
        results.name = args.name
    else:
        results.name = path.basename(args.results_path)

    backend = backends.get_backend(args.backend)(
        args.results_path,
        file_fsync=opts.sync,
        junit_suffix=args.junit_suffix)
    backend.initialize(_create_metadata(args, results.name, opts))

    profile = framework.profile.merge_test_profiles(args.test_profile)
    profile.results_dir = args.results_path

    time_start = time.time()
    # Set the dmesg type
    if args.dmesg:
        profile.dmesg = args.dmesg
    profile.run(opts, args.log_level, backend)
    time_end = time.time()

    results.time_elapsed = time_end - time_start
    backend.finalize({'time_elapsed': results.time_elapsed})

    print('Thank you for running Piglit!\n'
          'Results have been written to ' + args.results_path)


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
    args = parser.parse_args(input_)
    _disable_windows_exception_messages()

    results = backends.load(args.results_path)
    opts = core.Options(concurrent=results.options['concurrent'],
                        exclude_filter=results.options['exclude_filter'],
                        include_filter=results.options['filter'],
                        execute=results.options['execute'],
                        valgrind=results.options['valgrind'],
                        dmesg=results.options['dmesg'],
                        sync=results.options['sync'])

    core.get_config(args.config_file)

    opts.env['PIGLIT_PLATFORM'] = results.options['platform']

    results.options['env'] = core.collect_system_info()
    results.options['name'] = results.name

    # Resume only works with the JSON backend
    backend = backends.get_backend('json')(
        args.results_path,
        file_fsync=opts.sync,
        file_start_count=len(results.tests) + 1)
    # Specifically do not initialize again, everything initialize does is done.

    # Don't re-run tests that have already completed, incomplete status tests
    # have obviously not completed.
    for name, result in results.tests.iteritems():
        if result['result'] != 'incomplete':
            opts.exclude_tests.add(name)

    profile = framework.profile.merge_test_profiles(results.options['profile'])
    profile.results_dir = args.results_path
    if opts.dmesg:
        profile.dmesg = opts.dmesg

    # This is resumed, don't bother with time since it wont be accurate anyway
    profile.run(opts, results.options['log_level'], backend)

    backend.finalize()

    print("Thank you for running Piglit!\n"
          "Results have been written to {0}".format(args.results_path))
