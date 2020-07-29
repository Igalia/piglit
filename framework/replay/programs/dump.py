# coding=utf-8
#
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
from framework.replay import dump_trace_images
from framework.replay import options
from framework.programs import parsers as piglit_parsers
from . import parsers


__all__ = ['dump']


def _dump_from_trace(args):
    options.OPTIONS.device_name = args.device_name
    options.OPTIONS.results_path = args.output

    if args.calls is not None:
        calls = args.calls.split(",")
    else:
        calls = []

    return dump_trace_images.dump_from_trace(args.file_path, args.output,
                                             calls)


@exceptions.handler
def dump(input_):
    """ Parser for replayer dump command """
    unparsed = piglit_parsers.parse_config(input_)[1]

    # Set the parent of the config to add the -f/--config message
    parser = argparse.ArgumentParser(parents=[piglit_parsers.CONFIG,
                                              parsers.DEVICE,
                                              parsers.RESULTS_PATH])
    parser.add_argument(
        '-c', '--calls',
        dest='calls',
        required=False,
        default=None,
        help=('a comma separated list of call numbers from the trace to dump '
              '(default: last frame).'))
    parser.add_argument(
        'file_path',
        help=('the path to the local trace file '
              'from which to dump images.'))
    parser.set_defaults(func=_dump_from_trace)

    args = parser.parse_args(unparsed)

    return 0 if args.func(args) else 1
