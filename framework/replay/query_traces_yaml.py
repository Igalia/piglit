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

import yaml

from framework import exceptions
from framework.replay.trace_utils import trace_type_from_filename, trace_type_from_name


__all__ = ['download_url',
           'load_yaml',
           'trace_checksum',
           'trace_devices',
           'traces']


def load_yaml(y):
    try:
        return yaml.safe_load(y) or {}
    except yaml.YAMLError:
        raise exceptions.PiglitFatalError(
            'Cannot use the provided stream. Is it YAML?')


def trace_devices(trace):
    try:
        return [e['device'] for e in trace['expectations']]
    except KeyError:
        return []


def trace_checksum(trace, device_name):
    try:
        expectation = next(e for e in trace['expectations']
                           if e['device'] == device_name)

        return expectation['checksum']
    except StopIteration:
        return ''
    except KeyError:
        return ''


def download_url(y):
    try:
        return y['traces-db']['download-url'] if 'traces-db' in y else None
    except KeyError:
        return None


def traces(y, trace_types=None, device_name=None, checksum=False):
    traces = y.get('traces', []) or []

    if trace_types is not None:
        split_trace_types = [trace_type_from_name(t) for t
                             in trace_types.split(',')]
        traces = filter(lambda t: trace_type_from_filename(t['path'])
                        in split_trace_types, traces)
    if device_name is not None:
        traces = [t for t in traces if device_name in trace_devices(t)]

    traces = list(traces)

    result = list()
    if checksum:
        for t in traces:
            try:
                yield {'path': t['path'],
                       'checksum': trace_checksum(t, device_name)}
            except KeyError:
                yield {}
    else:
        for t in traces:
            try:
                yield {'path': t['path']}
            except KeyError:
                yield {}
