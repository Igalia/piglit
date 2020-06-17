#!/usr/bin/env python3
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
import sys
import yaml

import parsers
from traceutil import all_trace_type_names, trace_type_from_name
from traceutil import trace_type_from_filename


def trace_devices(trace):
    return [e['device'] for e in trace['expectations']]


def cmd_traces_db_download_url(args):
    y = yaml.safe_load(args.yaml_file)
    print(y['traces-db']['download-url'])


def cmd_traces(args):
    y = yaml.safe_load(args.yaml_file)

    traces = y['traces']
    split_trace_types = [trace_type_from_name(t) for t
                         in args.trace_types.split(",")]
    traces = filter(lambda t: trace_type_from_filename(t['path'])
                    in split_trace_types, traces)
    if args.device_name:
        traces = filter(lambda t: args.device_name in trace_devices(t), traces)

    traces = list(traces)

    if len(traces) == 0:
        return

    print('\n'.join((t['path'] for t in traces)))


def cmd_checksum(args):
    y = yaml.safe_load(args.yaml_file)

    traces = y['traces']
    trace = next(t for t in traces if t['path'] == args.trace_path)
    expectation = next(e for e in trace['expectations']
                       if e['device'] == args.device_name)

    print(expectation['checksum'])


def query(input_):
    """ Parser for tracie query command """
    parser = argparse.ArgumentParser(parents=[parsers.YAML])

    # Add a destination due to
    # https://github.com/python/cpython/pull/3027#issuecomment-330910633
    subparsers = parser.add_subparsers(dest='command', required=True)

    parser_traces_db_download_url = subparsers.add_parser(
        'traces_db_download_url')
    parser_traces_db_download_url.set_defaults(
        func=cmd_traces_db_download_url)

    parser_traces = subparsers.add_parser('traces', parents=[parsers.DEVICE])
    parser_traces.add_argument(
        '-t', '--trace-types',
        required=False,
        default=",".join(all_trace_type_names()),
        help=('the types of traces to look for in recursive dir walks '
              '(by default all types)'))
    parser_traces.set_defaults(func=cmd_traces)

    parser_checksum = subparsers.add_parser('checksum',
                                            parents=[parsers.DEVICE])
    parser_checksum.add_argument('trace_path')
    parser_checksum.set_defaults(func=cmd_checksum)

    args = parser.parse_args(input_)

    args.func(args)


def main():
    input_ = sys.argv[1:]

    query(input_)


if __name__ == "__main__":
    main()
