# coding=utf-8
#
# Copyright (c) 2014, 2016-2017, 2019-2020 Intel Corporation
# Copyright (c) 2019 Collabora Ltd
# Copyright © 2019-2020 Valve Corporation.
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


""" Module providing a trace testing dump backend for replayer """

import os
import subprocess

from os import path

from framework import core
from .abstract import DumpBackend, dump_handler
from .register import Registry


__all__ = [
    'REGISTRY',
    'TestTraceBackend',
]


class TestTraceBackend(DumpBackend):
    """ replayer's dump backend for trace testing

    This is a simple dump backend which reads a RGBA color from a text file and
    dumps a square image with that color.

    """
    _get_last_frame_call = None  # this silences the abstract-not-subclassed warning

    def __init__(self, trace_path, output_dir=None, calls=[], **kwargs):
        super(TestTraceBackend, self).__init__(trace_path, output_dir, calls,
                                               **kwargs)
        if len(self._calls) == 0: self._calls = ['0']

    @dump_handler
    def dump(self):
        from PIL import Image
        outputprefix = path.join(self._output_dir, path.basename(self._trace_path))
        with open(self._trace_path) as f:
            rgba = f.read()
        color = [int(rgba[0:2], 16), int(rgba[2:4], 16),
                 int(rgba[4:6], 16), int(rgba[6:8], 16)]
        for c in self._calls:
            outputfile = outputprefix + '-' + c + '.png'
            print('Writing RGBA: {} to {}'.format(rgba, outputfile))
            Image.frombytes('RGBA', (32, 32),
                            bytes(color * 32 * 32)).save(outputfile)


REGISTRY = Registry(
    extensions=['.testtrace'],
    backend=TestTraceBackend,
)
