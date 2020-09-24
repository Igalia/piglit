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

from framework import core
from .abstract import DumpBackend, dump_handler
from .register import Registry


__all__ = [
    'REGISTRY',
    'GFXReconstructBackend',
]


class GFXReconstructBackend(DumpBackend):
    """ replayer's GFXReconstruct dump backend

    This writes out to piglit's native json backend. This class uses the python
    json module or the simplejson.

    This class is atomic, writes either completely fail or completely succeed.
    To achieve this it writes individual files for each test and for the
    metadata, and composes them at the end into a single file and removes the
    intermediate files. When it tries to compose these files if it cannot read
    a file it just ignores it, making the result atomic.

    """

    def _get_last_frame_call(self):
        gfxrecon_info_bin = core.get_option(
            'PIGLIT_REPLAY_GFXRECON_INFO_BINARY',
            ('replay', 'gfxrecon-info_bin'),
            default='gfxrecon-info')
        cmd = [gfxrecon_info_bin, self._trace_path]
        ret = subprocess.run(cmd, stdout=subprocess.PIPE)
        lines = ret.stdout.decode(errors='replace').splitlines()
        print(ret.stdout.decode(errors='replace'))
        if len(lines) >= 1:
            c = lines[0].split(': ', 1)
            if len(c) >= 2 and c[1].isnumeric():
                return int(c[1])
        return -1

    @dump_handler
    def dump(self):
        super(GFXReconstructBackend, self).dump()
        from PIL import Image
        outputprefix = path.join(self._output_dir,
                                 path.basename(self._trace_path))
        if len(self._calls) == 0:
            # FIXME: The VK_LAYER_LUNARG_screenshot numbers the calls from 0 to
            # (total-num-calls - 1) while gfxreconstruct does it from 1 to
            # total-num-calls:
            # https://github.com/LunarG/gfxreconstruct/issues/284
            self._calls = [str(self._get_last_frame_call() - 1)]
        gfxrecon_replay_bin = core.get_option(
            'PIGLIT_REPLAY_GFXRECON_REPLAY_BINARY',
            ('replay', 'gfxrecon-replay_bin'),
            default='gfxrecon-replay')
        gfxrecon_replay_extra_args = core.get_option(
            'PIGLIT_REPLAY_GFXRECON_REPLAY_EXTRA_ARGS',
            ('replay', 'gfxrecon-replay_extra_args'),
            default='').split()
        cmd = ([gfxrecon_replay_bin]
               + gfxrecon_replay_extra_args + [self._trace_path])
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = ','.join(self._calls)
        env['VK_SCREENSHOT_DIR'] = self._output_dir
        self._run_logged_command(cmd, env)
        for c in self._calls:
            ppm = path.join(self._output_dir, c) + '.ppm'
            outputfile = outputprefix + '-' + c + '.png'
            print('Writing: {} to {}'.format(ppm, outputfile))
            Image.open(ppm).save(outputfile)
            os.remove(ppm)


REGISTRY = Registry(
    extensions=['.gfxr'],
    backend=GFXReconstructBackend,
)
