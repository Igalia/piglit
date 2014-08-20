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

import framework.core as core
import framework.results
import framework.profile

__all__ = ['run',
           'resume']


_PLATFORMS = ["glx", "x11_egl", "wayland", "gbm", "mixed_glx_egl"]


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
            if plat not in _PLATFORMS:
                print('Platform is not valid\n'
                      'valid platforms are: {}'.format(_PLATFORMS),
                      file=sys.stderr)
                sys.exit(1)
            return plat
        except (ConfigParser.NoOptionError, ConfigParser.NoSectionError):
            return 'mixed_glx_egl'


def _run_parser(input_):
    """ Parser for piglit run command """
    # Parse the config file before any other options, this allows the config
    # file to be used to sete default values for the parser.
    parser = argparse.ArgumentParser()
    parser.add_argument("-f", "--config",
                        dest="config_file",
                        type=argparse.FileType("r"),
                        help="Optionally specify a piglit config file to use. "
                             "Default is piglit.conf")
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
                        choices=_PLATFORMS,
                        default=_default_platform(),
                        help="Name of windows system passed to waffle")
    parser.add_argument("--valgrind",
                        action="store_true",
                        help="Run tests in valgrind's memcheck")
    parser.add_argument("--dmesg",
                        action="store_true",
                        help="Capture a difference in dmesg before and "
                             "after each test. Implies -1/--no-concurrency")
    parser.add_argument("-v", "--verbose",
                        action="store_true",
                        help="Produce a line of output for each test before "
                             "and after it runs")
    parser.add_argument("-s", "--sync",
                        action="store_true",
                        help="Sync results to disk after every test")
    parser.add_argument("test_profile",
                        metavar="<Path to one or more test profile(s)>",
                        nargs='+',
                        help="Path to testfile to run")
    parser.add_argument("results_path",
                        type=path.realpath,
                        metavar="<Results Path>",
                        help="Path to results folder")
    return parser.parse_args(unparsed)


def run(input_):
    """ Function for piglit run command

    This is a function because it allows it to be shared between piglit-run.py
    and piglit run

    """
    args = _run_parser(input_)

    # Disable Windows error message boxes for this and all child processes.
    if sys.platform == 'win32':
        # This disables messages boxes for uncaught exceptions, but it will not
        # disable the message boxes for assertion failures or abort().  Those
        # are created not by the system but by the CRT itself, and must be
        # disabled by the child processes themselves.
        import ctypes
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

    # If dmesg is requested we must have serial run, this is becasue dmesg
    # isn't reliable with threaded run
    if args.dmesg:
        args.concurrency = "none"

    # Pass arguments into Options
    opts = core.Options(concurrent=args.concurrency,
                        exclude_filter=args.exclude_tests,
                        include_filter=args.include_tests,
                        execute=args.execute,
                        valgrind=args.valgrind,
                        dmesg=args.dmesg,
                        verbose=args.verbose,
                        sync=args.sync)

    # Set the platform to pass to waffle
    opts.env['PIGLIT_PLATFORM'] = args.platform

    # Change working directory to the root of the piglit directory
    piglit_dir = path.dirname(path.realpath(sys.argv[0]))
    os.chdir(piglit_dir)
    core.checkDir(args.results_path, False)

    results = framework.results.TestrunResult()

    # Set results.name
    if args.name is not None:
        results.name = args.name
    else:
        results.name = path.basename(args.results_path)

    # Create a dictionary to pass to initialize json, it needs the contents of
    # the env dictionary and profile and platform information
    options = {'profile': args.test_profile}
    for key, value in opts:
        options[key] = value
    if args.platform:
        options['platform'] = args.platform
    options['name'] = results.name
    options['env'] = core.collect_system_info()

    # Begin json.
    result_filepath = path.join(args.results_path, 'results.json')
    json_writer = framework.results.JSONWriter(
        result_filepath,
        options,
        file_fsync=opts.sync)

    profile = framework.profile.merge_test_profiles(args.test_profile)
    profile.results_dir = args.results_path

    time_start = time.time()
    # Set the dmesg type
    if args.dmesg:
        profile.dmesg = args.dmesg
    profile.run(opts, json_writer)
    time_end = time.time()

    results.time_elapsed = time_end - time_start
    json_writer.finalize({'time_elapsed': results.time_elapsed})

    print('Thank you for running Piglit!\n'
          'Results have been written to ' + result_filepath)


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

    results = framework.results.load_results(args.results_path)
    opts = core.Options(concurrent=results.options['concurrent'],
                        exclude_filter=results.options['exclude_filter'],
                        include_filter=results.options['filter'],
                        execute=results.options['execute'],
                        valgrind=results.options['valgrind'],
                        dmesg=results.options['dmesg'],
                        verbose=results.options['verbose'],
                        sync=results.options['sync'])

    core.get_config(args.config_file)

    opts.env['PIGLIT_PLATFORM'] = results.options['platform']

    results.options['env'] = core.collect_system_info()
    results_path = path.join(args.results_path, 'results.json')
    json_writer = framework.results.JSONWriter(results_path,
                                               results.options,
                                               file_fsync=opts.sync)

    for key, value in results.tests.iteritems():
        json_writer.write_test(key, value)
        opts.exclude_tests.add(key)

    profile = framework.profile.merge_test_profiles(results.options['profile'])
    profile.results_dir = args.results_path
    if opts.dmesg:
        profile.dmesg = opts.dmesg

    # This is resumed, don't bother with time since it wont be accurate anyway
    profile.run(opts, json_writer)

    json_writer.finalize()

    print("Thank you for running Piglit!\n"
          "Results have ben wrriten to {0}".format(results_path))
