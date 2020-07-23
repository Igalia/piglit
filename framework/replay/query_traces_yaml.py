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

import framework.replay.parsers
from framework.replay.trace_utils import trace_type_from_filename, trace_type_from_name


def load_yaml(y):
    return yaml.safe_load(y) or {}


def trace_devices(trace):
    return [e['device'] for e in trace['expectations']]


def trace_checksum(trace, device_name):
    try:
        expectation = next(e for e in trace['expectations']
                           if e['device'] == device_name)

        return expectation['checksum']
    except StopIteration:
        return ""
    except KeyError:
        return ""


def download_url(y):
    return y["traces-db"]["download-url"] if "traces-db" in y else None


def traces(y, trace_types, device_name, checksum):
    traces = y.get('traces', []) or []

    if trace_types != '':
        split_trace_types = [trace_type_from_name(t) for t
                             in trace_types.split(",")]
        traces = filter(lambda t: trace_type_from_filename(t['path'])
                        in split_trace_types, traces)
    if device_name:
        traces = [t for t in traces if device_name in trace_devices(t)]

    traces = list(traces)

    result = list()
    if checksum:
        for t in traces:
            result.append({'path': t['path'],
                           'checksum': trace_checksum(t, device_name)})
    else:
        for t in traces:
            result.append({'path': t['path']})

    return result


def cmd_traces_db_download_url(args):
    y = load_yaml(args.yaml_file)

    url = download_url(y) or ""

    print(url)


def cmd_traces(args):
    y = load_yaml(args.yaml_file)

    t_list = traces(y, args.trace_types, args.device_name, args.checksum)

    if args.checksum:
        print('\n'.join(((t['path'] + '\n' + t['checksum'])
                         for t in t_list)))
    else:
        print('\n'.join((t['path'] for t in t_list)))


def cmd_checksum(args):
    y = load_yaml(args.yaml_file)

    traces = y['traces']
    try:
        trace = next(t for t in traces if t['path'] == args.trace_path)
    except StopIteration:
        print("")
        return

    print(trace_checksum(trace, args.device_name))


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
        default='',
        help=('a comma separated list of trace types to look for '
              'in recursive dir walks. '
              'If none are provide, all types are used by default'))
    parser_traces.add_argument(
        '-c', '--checksum',
        required=False,
        action='store_true',
        help='whether to print the checksum below every trace')
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
