# coding=utf-8
#
# Copyright (c) 2019 Collabora Ltd
# Copyright © 2020 Valve Corporation.
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#
# SPDX-License-Identifier: MIT

import argparse

from framework import exceptions
from framework.replay import query_traces_yaml as qty
from . import parsers


__all__ = ['query']


def _traces_db_download_url(args):
    y = qty.load_yaml(args.yaml_file)

    url = qty.download_url(y) or ""

    print(url)


def _traces(args):
    y = qty.load_yaml(args.yaml_file)

    t_list = qty.traces(y,
                        trace_extensions=args.trace_extensions,
                        device_name=args.device_name,
                        checksum=args.checksum)

    if args.checksum:
        print('\n'.join(((t['path'] + '\n' + t['checksum'])
                         for t in t_list)))
    else:
        print('\n'.join((t['path'] for t in t_list)))


def _checksum(args):
    y = qty.load_yaml(args.yaml_file)

    traces = y['traces']
    try:
        trace = next(t for t in traces if t['path'] == args.file_path)
    except StopIteration:
        print("")
        return

    print(qty.trace_checksum(trace, args.device_name))


@exceptions.handler
def query(input_):
    """ Parser for replayer query command """
    try:
        parser = argparse.ArgumentParser(parents=[parsers.YAML])
        # The "required" keyword is only available since python >= 3.7
        subparsers = parser.add_subparsers(dest='command', required=True)
    except TypeError:
        parser = argparse.ArgumentParser(parents=[parsers.YAML])
        # Add a destination due to
        # https://github.com/python/cpython/pull/3027#issuecomment-330910633
        subparsers = parser.add_subparsers(dest='command')

    parser_checksum = subparsers.add_parser(
        'checksum',
        parents=[parsers.DEVICE],
        help=('Outputs, if available, the checksum that a specific device '
              'should produce for a specified trace file.'))
    parser_checksum.add_argument(
        'file_path',
        help=('the path to a trace file.'))
    parser_checksum.set_defaults(func=_checksum)

    parser_traces = subparsers.add_parser(
        'traces',
        parents=[parsers.DEVICE],
        help=('Outputs the trace files filtered by the optional arguments.'))
    parser_traces.add_argument(
        '-t', '--trace-extensions',
        required=False,
        default=None,
        help=('a comma separated list of trace extensions to look for '
              'in recursive dir walks (e.g. ".trace,.rdc"). '
              'If none are provide, all supported extensions '
              'are used by default'))
    parser_traces.add_argument(
        '-c', '--checksum',
        action='store_true',
        help='whether to print the checksum below every trace')
    parser_traces.set_defaults(func=_traces)

    parser_traces_db_download_url = subparsers.add_parser(
        'traces_db_download_url',
        help=('Outputs, if available, the download URL.'))
    parser_traces_db_download_url.set_defaults(func=_traces_db_download_url)

    args = parser.parse_args(input_)

    args.func(args)
