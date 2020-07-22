#!/usr/bin/env python3
# coding=utf-8
#
# Copyright (c) 2014, 2019 Intel Corporation
# Copyright © 2020 Valve Corporation.
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
#
# SPDX-License-Identifier: MIT


""" Wrapper for replayer executable

This imports functions from the replay framework and calls them with the
argument parts that the parser defined here doesn't know how to parse.

It is very important that the final parser not generate a help message
(add_help=False in the constructor arguments), otherwise this parser will
capture -h/--help and the results will not be useful.

"""

import argparse
import os
import os.path as path
import sys


def setup_module_search_path():
    """Add Piglit's data directory to Python's module search path.

    This enables Python to import Piglit's framework module.

    CAUTION: This script must import the framework that *belongs to this
    script*. Mayhem occurs if this script accidentally imports a framework
    module that belongs to a different Piglit source tree or belongs to
    a different Piglit installation.

    CAUTION: This script file must be located in the Piglit source tree or in
    an installed location.  Otherwise this function may fail to find the
    framework module or, worse, it may succeed in finding a different Piglit's
    framework module.
    """

    piglit_data_dir = os.environ.get('PIGLIT_SOURCE_DIR', None)
    if piglit_data_dir is None:
        print('error: the PIGLIT_SOURCE_DIR env variable is not set. '
              'exiting ...', file=sys.stderr)
        sys.exit(1)

    if path.exists(path.join(piglit_data_dir, 'framework')):
        sys.path.append(piglit_data_dir)
        return

    print('error: failed to find piglit data directory. '
          'exiting...',file=sys.stderr)
    sys.exit(1)


setup_module_search_path()
import framework.replay.programs.compare as compare
import framework.replay.programs.query as query
import framework.replay.programs.download as download
import framework.replay.programs.checksum as checksum
import framework.replay.programs.dump as dump
from framework.replay.compare_replay import Result


def main():
    """ Parse argument and call other executables """
    input_ = sys.argv[1:]

    parser = argparse.ArgumentParser()

    # Add a destination due to
    # https://github.com/python/cpython/pull/3027#issuecomment-330910633
    subparsers = parser.add_subparsers(dest='command', required=True)

    parser_checksum = subparsers.add_parser(
        'checksum',
        add_help=False,
        help=('Calculates the checksums of a local image file.'))
    parser_checksum.set_defaults(func=checksum.checksum)

    parser_compare = subparsers.add_parser(
        'compare',
        add_help=False,
        help=('Trace replay compare methods.'))
    parser_compare.set_defaults(func=compare.compare)

    parser_download = subparsers.add_parser(
        'download',
        add_help=False,
        help=('Downloads a remote file into the db path.'))
    parser_download.set_defaults(func=download.download)

    parser_dump = subparsers.add_parser(
        'dump',
        add_help=False,
        help=('Dumps image(s) from a trace file.'))
    parser_dump.set_defaults(func=dump.dump)

    parser_query = subparsers.add_parser(
        'query',
        add_help=False,
        help=('Queries for specific information '
              'from a traces description file listing traces '
              'and their checksums for each device.'))
    parser_query.set_defaults(func=query.query)

    # Parse the known arguments (replayer compare or replayer query for
    # example), and then pass the arguments that this parser doesn't know about
    # to that executable
    parsed, args = parser.parse_known_args(input_)
    try:
        runner = parsed.func
    except AttributeError:
        parser.print_help()
        sys.exit(1)

    result = runner(args)
    if result is not Result.MATCH:
        sys.exit(1 if result is Result.FAILURE else 2)


if __name__ == '__main__':
    main()
