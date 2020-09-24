# coding=utf-8
#
# Copyright (c) 2014, 2016-2017, 2019-2020 Intel Corporation
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


""" Module providing a RenderDoc dump backend for replayer """

from os import path

from framework import exceptions
from .abstract import DumpBackend, dump_handler
from .register import Registry


__all__ = [
    'REGISTRY',
    'RenderDocBackend',
]


class RenderDocBackend(DumpBackend):
    """ replayer's RenderDoc dump backend

    This backend uses RenderDoc for replaying its traces.

    """
    _get_last_frame_call = None  # this silences the abstract-not-subclassed warning

    def __init__(self, trace_path, output_dir=None, calls=None, **kwargs):
        super(RenderDocBackend, self).__init__(trace_path, output_dir, calls,
                                               **kwargs)
        extension = path.splitext(self._trace_path)[1]

        if extension != '.rdc':
            raise exceptions.PiglitFatalError(
                'Invalid trace_path: "{}" tried to be dumped '
                'by the RenderDocBackend.\n'.format(self._trace_path))

    @dump_handler
    def dump(self):
        script_path = path.dirname(path.abspath(__file__))
        cmd = [path.join(script_path, 'renderdoc/renderdoc_dump_images.py'),
               self._trace_path, self._output_dir]
        cmd.extend(self._calls)
        self._run_logged_command(cmd, None)


REGISTRY = Registry(
    extensions=['.rdc'],
    backend=RenderDocBackend,
)
