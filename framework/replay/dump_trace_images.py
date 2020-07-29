# coding=utf-8
#
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

import os
import subprocess

from os import path

from framework import core
from framework.replay.options import OPTIONS
from framework.replay.trace_utils import trace_type_from_filename, TraceType


__all__ = ['dump_from_trace']


def _log(severity, msg, end='\n'):
    print('[dump_trace_images] {}: {}'.format(severity, msg), flush=True,
          end=end)

def _log_result(msg):
    print(msg, flush=True)

def _run_logged_command(cmd, env, log_path):
    ret = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, env=env)
    logoutput = '[dump_trace_images] Running: {}\n'.format(
        ' '.join(cmd)).encode() + ret.stdout
    core.check_dir(path.dirname(log_path))
    with open(log_path, 'wb') as log:
        log.write(logoutput)
    if ret.returncode:
        raise RuntimeError(
            logoutput.decode(errors='replace') +
            '[dump_traces_images] Process failed with error code: {}'.format(
                ret.returncode))

def _get_last_apitrace_frame_call(cmd_wrapper, trace_path):
    if len(cmd_wrapper) > 0:
        apitrace_bin = core.get_option('PIGLIT_REPLAY_WINE_APITRACE_BINARY',
                                       ('replay', 'wine_apitrace_bin'),
                                       default='apitrace')
    else:
        apitrace_bin = core.get_option('PIGLIT_REPLAY_APITRACE_BINARY',
                                       ('replay', 'apitrace_bin'),
                                       default='apitrace')
    cmd = cmd_wrapper + [apitrace_bin, 'dump', '--calls=frame', trace_path]
    ret = subprocess.run(cmd, stdout=subprocess.PIPE)
    for l in reversed(ret.stdout.decode(errors='replace').splitlines()):
        s = l.split(None, 1)
        if len(s) >= 1 and s[0].isnumeric():
            return int(s[0])
    return -1

def _get_last_gfxreconstruct_frame_call(trace_path):
    gfxrecon_info_bin = core.get_option('PIGLIT_REPLAY_GFXRECON_INFO_BINARY',
                                        ('replay', 'gfxrecon-info_bin'),
                                        default='gfxrecon-info')
    cmd = [gfxrecon_info_bin, trace_path]
    ret = subprocess.run(cmd, stdout=subprocess.PIPE)
    lines = ret.stdout.decode(errors='replace').splitlines()
    if len(lines) >= 1:
        c = lines[0].split(': ', 1)
        if len(c) >= 2 and c[1].isnumeric():
            return int(c[1])
    return -1

def _dump_with_apitrace(retrace_cmd, trace_path, output_dir, calls):
    outputprefix = path.join(output_dir, path.basename(trace_path)) + '-'
    if len(calls) == 0:
        calls = [str(_get_last_apitrace_frame_call(retrace_cmd[:-1],
                                                   trace_path))]
    cmd = retrace_cmd + ['--headless',
                         '--snapshot=' + ','.join(calls),
                         '--snapshot-prefix=' + outputprefix,
                         trace_path]
    log_path = path.join(output_dir, path.basename(trace_path)) + '.log'
    _run_logged_command(cmd, None, log_path)

def _dump_with_renderdoc(trace_path, output_dir, calls):
    script_path = path.dirname(path.abspath(__file__))
    cmd = [path.join(script_path, 'renderdoc_dump_images.py'),
           trace_path, output_dir]
    cmd.extend(calls)
    log_path = path.join(output_dir, path.basename(trace_path)) + '.log'
    _run_logged_command(cmd, None, log_path)

def _dump_with_gfxreconstruct(trace_path, output_dir, calls):
    from PIL import Image
    outputprefix = path.join(output_dir, path.basename(trace_path))
    if len(calls) == 0:
        # FIXME: The VK_LAYER_LUNARG_screenshot numbers the calls from
        # 0 to (total-num-calls - 1) while gfxreconstruct does it from
        # 1 to total-num-calls:
        # https://github.com/LunarG/gfxreconstruct/issues/284
        calls = [str(_get_last_gfxreconstruct_frame_call(trace_path) - 1)]
    gfxrecon_replay_bin = core.get_option(
        'PIGLIT_REPLAY_GFXRECON_REPLAY_BINARY',
        ('replay', 'gfxrecon-replay_bin'),
        default='gfxrecon-replay')
    cmd = [gfxrecon_replay_bin, trace_path]
    env = os.environ.copy()
    env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
    env['VK_SCREENSHOT_FRAMES'] = ','.join(calls)
    env['VK_SCREENSHOT_DIR'] = output_dir
    log_path = outputprefix + '.log'
    _run_logged_command(cmd, env, log_path)
    for c in calls:
        ppm = path.join(output_dir, c) + '.ppm'
        outputfile = outputprefix + '-' + c + '.png'
        with open(log_path, 'a') as log:
            log.write('Writing: {} to {}'.format(ppm, outputfile))
        Image.open(ppm).save(outputfile)
        os.remove(ppm)

def _dump_with_testtrace(trace_path, output_dir, calls):
    from PIL import Image
    outputprefix = path.join(output_dir, path.basename(trace_path))
    with open(trace_path) as f:
        rgba = f.read()
    color = [int(rgba[0:2], 16), int(rgba[2:4], 16),
             int(rgba[4:6], 16), int(rgba[6:8], 16)]
    if len(calls) == 0: calls = ['0']
    log_path = outputprefix + '.log'
    for c in calls:
        outputfile = outputprefix + '-' + c + '.png'
        with open(log_path, 'w') as log:
            log.write('Writing RGBA: {} to {}'.format(rgba, outputfile))
        Image.frombytes('RGBA', (32, 32),
                        bytes(color * 32 * 32)).save(outputfile)

def dump_from_trace(trace_path, output_dir=None, calls=[]):
    _log('Info', 'Dumping trace {}'.format(trace_path), end='...\n')
    if output_dir is None:
        output_dir = path.join('trace', OPTIONS.device_name,
                               path.dirname(trace_path))
    core.check_dir(output_dir)
    trace_type = trace_type_from_filename(path.basename(trace_path))
    try:
        if trace_type == TraceType.APITRACE:
            eglretrace_bin = core.get_option('PIGLIT_REPLAY_EGLRETRACE_BINARY',
                                             ('replay', 'eglretrace-d_bin'),
                                             default='eglretrace')
            _dump_with_apitrace([eglretrace_bin],
                                trace_path, output_dir, calls)
        elif trace_type == TraceType.APITRACE_DXGI:
            wine_bin = core.get_option('PIGLIT_REPLAY_WINE_BINARY',
                                       ('replay', 'wine_bin'),
                                       default='wine')
            wine_d3dretrace_bin = core.get_option(
                'PIGLIT_REPLAY_WINE_D3DRETRACE_BINARY',
                ('replay', 'wine_d3dretrace_bin'),
                default='d3dretrace')
            _dump_with_apitrace([wine_bin, wine_d3dretrace_bin],
                                trace_path, output_dir, calls)
        elif trace_type == TraceType.RENDERDOC:
            _dump_with_renderdoc(trace_path, output_dir, calls)
        elif trace_type == TraceType.GFXRECONSTRUCT:
            _dump_with_gfxreconstruct(trace_path, output_dir, calls)
        elif trace_type == TraceType.TESTTRACE:
            _dump_with_testtrace(trace_path, output_dir, calls)
        else:
            raise RuntimeError('Unknown tracefile extension')
        _log_result('OK')
        return True
    except Exception as e:
        _log_result('ERROR')
        _log('Debug', '=== Failure log start ===')
        print(e)
        _log('Debug', '=== Failure log end ===')
        return False
