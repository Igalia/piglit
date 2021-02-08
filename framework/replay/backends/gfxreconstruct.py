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


""" Module providing a GFXReconstruct dump backend for replayer """

import os
import re
import subprocess

from packaging import version
from os import path

from framework import core, exceptions
from . import DumpBackendError
from .abstract import DumpBackend, dump_handler
from .register import Registry


__all__ = [
    'REGISTRY',
    'GFXReconstructBackend',
]


_MIN_VERSION = version.Version('0.9.4')
_VERSION_RE = re.compile('\s*GFXReconstruct Version\s*([0-9]\.[0-9]\.[0-9])')

_TOTAL_FRAMES_RE = re.compile('\s*Total frames:\s*([0-9]*)')

class GFXReconstructBackend(DumpBackend):
    """ replayer's GFXReconstruct dump backend

    This backend uses GFXReconstruct for replaying its traces.

    The path to the GFXReconstruct binary is configurable.

    It also admits configuration for passing extra parameters to
    GFXReconstruct.

    """

    def __init__(self, trace_path, output_dir=None, calls=None, **kwargs):
        super(GFXReconstructBackend, self).__init__(trace_path, output_dir,
                                                    calls, **kwargs)
        extension = path.splitext(self._trace_path)[1]

        if extension != '.gfxr':
            raise exceptions.PiglitFatalError(
                'Invalid trace_path: "{}" tried to be dumped '
                'by the GFXReconstructBackend.\n'.format(self._trace_path))

    def _get_last_frame_call(self):
        gfxrecon_info_bin = core.get_option(
            'PIGLIT_REPLAY_GFXRECON_INFO_BINARY',
            ('replay', 'gfxrecon-info_bin'),
            default='gfxrecon-info')
        cmd = [gfxrecon_info_bin, self._trace_path]
        ret = subprocess.run(cmd, stdout=subprocess.PIPE)
        lines = ret.stdout.decode(errors='replace').splitlines()
        print(ret.stdout.decode(errors='replace'))
        try:
            frames = re.search(_TOTAL_FRAMES_RE, lines[2])
            return int(frames.group(1))
        except:
            return -1

    def _check_version(self, gfxrecon_replay_bin):
        cmd = [gfxrecon_replay_bin, '--version']
        ret = subprocess.run(cmd, stdout=subprocess.PIPE)
        lines = ret.stdout.decode(errors='replace').splitlines()
        print(ret.stdout.decode(errors='replace'))
        try:
            v = re.search(_VERSION_RE, lines[1])
            current = version.Version(v.group(1))
        except:
            raise DumpBackendError(
                '[dump_trace_images] Unable to check the current '
                'gfxrecon-replay version.')
        if _MIN_VERSION > current:
            raise DumpBackendError(
                '[dump_trace_images] The current gfxrecon-replay version '
                'is {}. Try to update, at least to the {} version.'.format(
                    current, _MIN_VERSION))

    @dump_handler
    def dump(self):
        from PIL import Image
        outputprefix = path.join(self._output_dir,
                                 path.basename(self._trace_path))
        gfxrecon_replay_bin = core.get_option(
            'PIGLIT_REPLAY_GFXRECON_REPLAY_BINARY',
            ('replay', 'gfxrecon-replay_bin'),
            default='gfxrecon-replay')
        self._check_version(gfxrecon_replay_bin)
        if not self._calls:
            self._calls = [str(self._get_last_frame_call())]
        gfxrecon_replay_extra_args = core.get_option(
            'PIGLIT_REPLAY_GFXRECON_REPLAY_EXTRA_ARGS',
            ('replay', 'gfxrecon-replay_extra_args'),
            default='').split()
        cmd = ([gfxrecon_replay_bin] + gfxrecon_replay_extra_args +
               ['--screenshots', ','.join(self._calls),
                '--screenshot-dir', self._output_dir,
                self._trace_path])
        self._run_logged_command(cmd, None)
        for c in self._calls:
            bmp = '{}_frame_{}.bmp'.format(path.join(self._output_dir,
                                                    'screenshot'),
                                          c)
            outputfile = '{}-{}.png'.format(outputprefix, c)
            print('Writing: {} to {}'.format(bmp, outputfile))
            Image.open(bmp).save(outputfile)
            os.remove(bmp)


REGISTRY = Registry(
    extensions=['.gfxr'],
    backend=GFXReconstructBackend,
)
