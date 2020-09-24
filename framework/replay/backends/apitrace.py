# coding=utf-8
#
# Copyright (c) 2014, 2016-2017, 2019-2020 Intel Corporation
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


""" Module providing json backend for piglit """

import os
import subprocess

from os import path

from framework import core, exceptions
from .abstract import DumpBackend, dump_handler
from .register import Registry


__all__ = [
    'REGISTRY',
    'APITraceBackend',
]


class APITraceBackend(DumpBackend):
    """ replayer's GFXReconstruct dump backend

    This writes out to piglit's native json backend. This class uses the python
    json module or the simplejson.

    This class is atomic, writes either completely fail or completely succeed.
    To achieve this it writes individual files for each test and for the
    metadata, and composes them at the end into a single file and removes the
    intermediate files. When it tries to compose these files if it cannot read
    a file it just ignores it, making the result atomic.

    """

    def __init__(self, trace_path, output_dir=None, calls=[], **kwargs):
        super(TestTraceBackend, self).__init__(trace_path, output_dir, calls,
                                               **kwargs)
        name, extension = self._trace_path

        if '.trace' == extension:
            eglretrace_bin = core.get_option('PIGLIT_REPLAY_EGLRETRACE_BINARY',
                                             ('replay', 'eglretrace-d_bin'),
                                             default='eglretrace')
            self._retrace_cmd = [eglretrace_bin]
        elif '.trace-dxgi' == extension:
            wine_bin = core.get_option('PIGLIT_REPLAY_WINE_BINARY',
                                       ('replay', 'wine_bin'),
                                       default='wine')
            wine_d3dretrace_bin = core.get_option(
                'PIGLIT_REPLAY_WINE_D3DRETRACE_BINARY',
                ('replay', 'wine_d3dretrace_bin'),
                default='d3dretrace')
            self._retrace_cmd = [wine_bin, wine_d3dretrace_bin]
        else:
            raise exceptions.PiglitFatalError(
                'Invalid trace_path: "{}" tried to be dumped '
                'by the APITraceBackend.\n'.format(self._trace_path))


    def _get_last_frame_call(self):
        cmd_wrapper = self._retrace_cmd[:-1]
        if len(cmd_wrapper) > 0:
            apitrace_bin = core.get_option(
                'PIGLIT_REPLAY_WINE_APITRACE_BINARY',
                ('replay', 'wine_apitrace_bin'),
                default='apitrace')
        else:
            apitrace_bin = core.get_option('PIGLIT_REPLAY_APITRACE_BINARY',
                                           ('replay', 'apitrace_bin'),
                                           default='apitrace')
        cmd = cmd_wrapper + [apitrace_bin,
                             'dump', '--calls=frame', self._trace_path]
        ret = subprocess.run(cmd, stdout=subprocess.PIPE)
        logoutput = '[dump_trace_images] Running: {}\n'.format(
            ' '.join(cmd)) + ret.stdout.decode(errors='replace')
        print(logoutput)
        for l in reversed(ret.stdout.decode(errors='replace').splitlines()):
            s = l.split(None, 1)
            if len(s) >= 1 and s[0].isnumeric():
                return int(s[0])
        return -1

    @dump_handler
    def dump(self):
        super(APITraceBackend, self).dump()
        outputprefix = path.join(self._output_dir,
                                 path.basename(self._trace_path)) + '-'
        if len(self._calls) == 0:
            self._calls = [str(self._get_last_frame_call())]
        cmd = retrace_cmd + ['--headless',
                             '--snapshot=' + ','.join(self._calls),
                             '--snapshot-prefix=' + outputprefix,
                             self._trace_path]
        self._run_logged_command(cmd, None)


REGISTRY = Registry(
    extensions=['.trace', '.trace-dxgi'],
    backend=APITraceBackend,
)
