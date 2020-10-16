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


"""Tests for replayer's gfxreconstruct backend."""

import pytest

import os
import subprocess
import textwrap

from os import path

from framework import core
from framework import exceptions
from framework.replay import backends
from framework.replay.options import OPTIONS


@pytest.yield_fixture
def config(mocker):
    conf = mocker.patch('framework.core.PIGLIT_CONFIG',
                        new_callable=core.PiglitConfig)
    conf.add_section('replay')
    yield conf

class TestGFXReconstructBackend(object):
    """Tests for the GFXReconstructBackend class."""

    def mock_gfxreconstruct_subprocess_run(self, cmd, stdout, env=None):
        if cmd[0].endswith(self.gfxrecon_info):
            # VK get_last_call
            ret = subprocess.CompletedProcess(cmd, 0)
            if cmd[-1] == self.vk_last_frame_fails_trace_path:
                ret.stdout = b''
            else:
                ret.stdout = bytearray('Total frames: ' +
                                       str(int(self.vk_trace_last_call) + 1) +
                                       textwrap.dedent('''
                                       Application info:
                                               Application name: vkcube
                                               Application version: 0
                                               Engine name: vkcube
                                               Engine version: 0
                                               Target API version: 4194304 (1.0.0)

                                       Physical device info:
                                               Device name: TESTING DEVICE
                                               Device ID: 0xabcd
                                               Vendor ID: 0xefgh
                                               Driver version: 83890275 (0x5001063)
                                               API version: 4202627 (1.2.131)

                                       Device memory allocation info:
                                               Total allocations: 5
                                               Min allocation size: 1216
                                               Max allocation size: 540680

                                       Pipeline info:
                                               Total graphics pipelines: 1
                                               Total compute pipelines: 0
                                       '''),
                                       'utf-8')

            return ret
        elif cmd[0].endswith(self.gfxrecon_replay):
            # VK replay
            ret = subprocess.CompletedProcess(cmd, 0)
            if cmd[-1] != self.vk_replay_crashes_trace_path:
                calls = env['VK_SCREENSHOT_FRAMES']
                prefix = env['VK_SCREENSHOT_DIR']
                ret.stdout = b''
                if env['VK_INSTANCE_LAYERS'] == 'VK_LAYER_LUNARG_screenshot':
                    for call in calls.split(','):
                        if (call != self.vk_trace_wrong_call and
                            call != '-1'):
                            from PIL import Image
                            dump = path.join(prefix, call + '.ppm')
                            rgba = 'ff00ffff'
                            color = [int(rgba[0:2], 16), int(rgba[2:4], 16),
                                     int(rgba[4:6], 16), int(rgba[6:8], 16)]
                            Image.frombytes('RGBA', (32, 32),
                                            bytes(color * 32 * 32)).save(dump)
                            ret.stdout += bytearray('Screen Capture file '
                                                    'is: ' + dump + '\n',
                                                    'utf-8')
                ret.stdout += bytearray('35.650065 fps, 0.280504 seconds, '
                                        '10 frames, 1 loop, framerange 1-10\n',
                                        'utf-8')
            else:
                ret.stdout = b'\n'
                ret.returncode = 1

            return ret
        else:
            raise exceptions.PiglitFatalError(
                'Non treated cmd: {}'.format(cmd))

    @pytest.fixture(autouse=True)
    def setup(self, mocker, tmpdir):
        """Setup for TestGFXReconstructBackend.

        This set ups the basic environment for testing.
        """

        OPTIONS.device_name = 'test-device'
        self.gfxrecon_info = 'gfxrecon-info'
        self.gfxrecon_replay = 'gfxrecon-replay'
        self.gfxrecon_replay_extra = ''
        self.vk_trace_path = tmpdir.mkdir(
            'db-path').join('KhronosGroup-Vulkan-Tools/vkcube.gfxr').strpath
        self.vk_last_frame_fails_trace_path = tmpdir.join(
            'db-path',
            'last-frame/fails.gfxr').strpath
        self.vk_replay_crashes_trace_path = tmpdir.join(
            'db-path',
            'replay/fails.gfxr').strpath
        self.vk_trace_calls = '2,5'
        self.vk_trace_last_call = '9'
        self.vk_trace_wrong_call = '10'
        self.output_dir = tmpdir.mkdir('results').strpath
        self.results_partial_path = path.join('trace', OPTIONS.device_name)
        self.m_gfxreconstruct_subprocess_run = mocker.patch(
            'framework.replay.backends.gfxreconstruct.subprocess.run',
            side_effect=self.mock_gfxreconstruct_subprocess_run)
        self.tmpdir = tmpdir
        self.mocker = mocker
        mocker.patch.dict('os.environ')
        self.env = os.environ
        self.env.clear()

    @pytest.mark.raises(exception=exceptions.PiglitFatalError)
    def test_init_unsupported_trace(self):
        """Tests for the init method.

        Should raise an exception in case of creating with an unsupported trace
        format.

        """
        test = backends.gfxreconstruct.GFXReconstructBackend(
            'unsupported_trace.trace')

    @pytest.mark.parametrize('option, gfxrecon_info, gfxrecon_replay', [
        (0, '/env/gfxrecon-info', '/env/gfxrecon-replay'),
        (1, '/config/gfxrecon-info', '/config/gfxrecon-replay'),
        (2, 'gfxrecon-info', 'gfxrecon-replay'),
    ])
    def test_dump_vk_options(self,
                             option, gfxrecon_info, gfxrecon_replay, config):
        """Tests for the dump method: basic with options.

        Check basic VK dumps with different configurations. No specific output
        directory is provided and leaving to the method to figure out the last
        call from the trace file itself.

        """
        self.gfxrecon_info = gfxrecon_info
        self.gfxrecon_replay = gfxrecon_replay
        if option == 0:
            self.env['PIGLIT_REPLAY_GFXRECON_INFO_BINARY'] = self.gfxrecon_info
            self.env[
                'PIGLIT_REPLAY_GFXRECON_REPLAY_BINARY'] = self.gfxrecon_replay
        elif option == 1:
            config.set('replay', 'gfxrecon-info_bin', self.gfxrecon_info)
            config.set('replay', 'gfxrecon-replay_bin', self.gfxrecon_replay)
        calls = self.vk_trace_last_call
        trace_path = self.vk_trace_path
        test = backends.gfxreconstruct.GFXReconstructBackend(trace_path)
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = calls
        env['VK_SCREENSHOT_DIR'] = path.dirname(trace_path)
        m_calls = [self.mocker.call(
            [self.gfxrecon_info, trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.gfxrecon_replay, trace_path],
                       env=env, stdout=subprocess.PIPE)]
        assert self.m_gfxreconstruct_subprocess_run.call_count == 2
        self.m_gfxreconstruct_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call + '.png')

    @pytest.mark.parametrize('option, gfxrecon_replay_extra', [
        (0, '-m remap'),
        (1, '-m rebind'),
    ])
    def test_dump_vk_replay_extra_args(self,
                                       option, gfxrecon_replay_extra, config):
        """Tests for the dump method: basic with replay extra args.

        Check basic VK dumps with different replay extra args. No specific
        output directory and other options are provided and it is left to the
        method to figure out the last call from the trace file itself.

        """
        if option == 0:
            self.env[
                'PIGLIT_REPLAY_GFXRECON_REPLAY_EXTRA_ARGS'
            ] = gfxrecon_replay_extra
        elif option == 1:
            config.set('replay', 'gfxrecon-replay_extra_args',
                       gfxrecon_replay_extra)
        calls = self.vk_trace_last_call
        trace_path = self.vk_trace_path
        test = backends.gfxreconstruct.GFXReconstructBackend(trace_path)
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = calls
        env['VK_SCREENSHOT_DIR'] = path.dirname(trace_path)
        m_calls = [self.mocker.call(
            [self.gfxrecon_info, trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.gfxrecon_replay] +
                       gfxrecon_replay_extra.split() + [trace_path],
                       env=env, stdout=subprocess.PIPE)]
        assert self.m_gfxreconstruct_subprocess_run.call_count == 2
        self.m_gfxreconstruct_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call + '.png')

    def test_dump_vk_output(self):
        """Tests for the dump method: explicit output directory.

        Check a basic VK dump, specifying the output and leaving for the method
        to figure out the last call from the trace file itself.

        """
        calls = self.vk_trace_last_call
        trace_path = self.vk_trace_path
        test = backends.gfxreconstruct.GFXReconstructBackend(
            trace_path, output_dir=self.output_dir)
        assert test.dump()
        snapshot_prefix = path.join(self.output_dir,
                                    path.basename(trace_path) + '-')
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = calls
        env['VK_SCREENSHOT_DIR'] = self.output_dir
        m_calls = [self.mocker.call(
            [self.gfxrecon_info, trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.gfxrecon_replay, trace_path],
                       env=env, stdout=subprocess.PIPE)]
        assert self.m_gfxreconstruct_subprocess_run.call_count == 2
        self.m_gfxreconstruct_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call + '.png')

    def test_dump_vk_calls(self):
        """Tests for the dump method: explicit valid calls.

        Check a basic VK dump, specifying valid calls to dump. No specific
        output directory is provided.

        """
        calls = self.vk_trace_calls
        trace_path = self.vk_trace_path
        test = backends.gfxreconstruct.GFXReconstructBackend(
            trace_path, calls=calls.split(','))
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = calls
        env['VK_SCREENSHOT_DIR'] = path.dirname(trace_path)
        self.m_gfxreconstruct_subprocess_run.assert_called_once_with(
            [self.gfxrecon_replay, trace_path],
            env=env, stdout=subprocess.PIPE)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call + '.png')

    def test_dump_vk_wrong_call(self):
        """Tests for the dump method: explicit invalid call.

        Check a basic VK dump, specifying an invalid call to dump. No specific
        output directory is provided.

        """
        calls = self.vk_trace_wrong_call
        trace_path = self.vk_trace_path
        test = backends.gfxreconstruct.GFXReconstructBackend(
            trace_path, calls=calls.split(','))
        assert not test.dump()
        snapshot_prefix = trace_path + '-'
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = calls
        env['VK_SCREENSHOT_DIR'] = path.dirname(trace_path)
        self.m_gfxreconstruct_subprocess_run.assert_called_once_with(
            [self.gfxrecon_replay, trace_path],
            env=env, stdout=subprocess.PIPE)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call + '.png')

    def test_dump_vk_last_frame_fails(self):
        """Tests for the dump method: the call to figure out the last frame fails.

        Check a basic VK dump. The call to figure out the last frame fails.

        """
        calls = '-1'
        trace_path = self.vk_last_frame_fails_trace_path
        test = backends.gfxreconstruct.GFXReconstructBackend(trace_path)
        assert not test.dump()
        snapshot_prefix = trace_path + '-'
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = calls
        env['VK_SCREENSHOT_DIR'] = path.dirname(trace_path)
        m_calls = [self.mocker.call(
            [self.gfxrecon_info, trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.gfxrecon_replay, trace_path],
                       env=env, stdout=subprocess.PIPE)]
        assert self.m_gfxreconstruct_subprocess_run.call_count == 2
        self.m_gfxreconstruct_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call + '.png')

    def test_dump_vk_replay_crashes(self):
        """Tests for the dump method: the replay call crashes.

        Check a basic VK dump. The replay call crashes.

        """
        calls = self.vk_trace_last_call
        trace_path = self.vk_replay_crashes_trace_path
        test = backends.gfxreconstruct.GFXReconstructBackend(trace_path)
        assert not test.dump()
        snapshot_prefix = trace_path + '-'
        env = os.environ.copy()
        env['VK_INSTANCE_LAYERS'] = 'VK_LAYER_LUNARG_screenshot'
        env['VK_SCREENSHOT_FRAMES'] = calls
        env['VK_SCREENSHOT_DIR'] = path.dirname(trace_path)
        m_calls = [self.mocker.call(
            [self.gfxrecon_info, trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.gfxrecon_replay, trace_path],
                       env=env, stdout=subprocess.PIPE)]
        assert self.m_gfxreconstruct_subprocess_run.call_count == 2
        self.m_gfxreconstruct_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call + '.png')
