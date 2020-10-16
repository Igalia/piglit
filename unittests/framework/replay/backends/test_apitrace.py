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


"""Tests for replayer's apitrace backend."""

import pytest

import os
import subprocess

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

class TestAPITraceBackend(object):
    """Tests for the APITraceBackend class."""

    def mock_apitrace_subprocess_run(self, cmd, stdout, env=None):
        get_last_call_args = ['dump', '--calls=frame']
        replay_retrace_args = ['--headless']
        if cmd[1:-1] == get_last_call_args:
            # GL get_last_call
            ret = subprocess.CompletedProcess(cmd, 0)
            if cmd[-1] == self.gl_last_frame_fails_trace_path:
                ret.stdout = b'\n'
            else:
                ret.stdout = bytearray(
                    str(int(self.gl_trace_last_call) - 50) +
                    ' glXSwapBuffers(dpy = 0x56060e921f80, '
                    'drawable = 31457282)\n'
                    '\n' +
                    self.gl_trace_last_call +
                    ' glXSwapBuffers(dpy = 0x56060e921f80, '
                    'drawable = 31457282)\n',
                    'utf-8')

            return ret
        elif cmd[1:2] == replay_retrace_args:
            # GL replay
            ret = subprocess.CompletedProcess(cmd, 0)
            if cmd[-1] == self.gl_trace_path:
                calls = cmd[2].split('=')[1]
                prefix = cmd[3].split('=')[1]
                ret.stdout = b''
                for call in calls.split(','):
                    if call != self.gl_trace_wrong_call:
                        dump = prefix + call.zfill(10) + '.png'
                        with open(dump, 'w') as f:
                            f.write("content")
                        ret.stdout += bytearray('Wrote ' + dump + '\n',
                                                'utf-8')
            else:
                ret.stdout = b'\n'
                if cmd[-1] == self.gl_replay_crashes_trace_path:
                    ret.returncode = 1

            return ret
        elif cmd[2:-1] == get_last_call_args:
            # DXGI get_last_call
            ret = subprocess.CompletedProcess(cmd, 0)
            if cmd[-1] == self.dxgi_last_frame_fails_trace_path:
                ret.stdout = b'\n'
            else:
                ret.stdout = bytearray(
                    str(int(self.dxgi_trace_last_call) - 50) +
                    ' IDXGISwapChain4::Present(this = 0x3de3b0, '
                    'SyncInterval = 1, Flags = 0x0) = S_OK\n'
                    '\n' +
                    self.dxgi_trace_last_call +
                    ' IDXGISwapChain4::Present(this = 0x3de3b0, '
                    'SyncInterval = 1, Flags = 0x0) = S_OK\n',
                    'utf-8')
            return ret
        elif cmd[2:3] == replay_retrace_args:
            # DXGI replay
            ret = subprocess.CompletedProcess(cmd, 0)
            if cmd[-1] == self.dxgi_trace_path:
                calls = cmd[3].split('=')[1]
                prefix = cmd[4].split('=')[1]
                ret.stdout = b''
                for call in calls.split(','):
                    dump = prefix + call.zfill(10) + '.png'
                    if call != self.dxgi_trace_wrong_call:
                        with open(dump, 'w') as f:
                            f.write("content")
                        ret.stdout += bytearray('Wrote ' + dump + '\n',
                                                'utf-8')
            else:
                ret.stdout = b'\n'
                if cmd[-1] == self.dxgi_replay_crashes_trace_path:
                    ret.returncode = 1

            return ret
        else:
            raise exceptions.PiglitFatalError(
                'Non treated cmd: {}'.format(cmd))

    @pytest.fixture(autouse=True)
    def setup(self, mocker, tmpdir):
        """Setup for TestAPITraceBackend.

        This set ups the basic environment for testing.
        """

        OPTIONS.device_name = 'test-device'
        self.apitrace = 'apitrace'
        self.eglretrace = 'eglretrace'
        self.wine = 'wine'
        self.d3dretrace = 'd3dretrace'
        self.gl_trace_path = tmpdir.mkdir(
            'db-path').join('glxgears/glxgears-2.trace').strpath
        self.gl_last_frame_fails_trace_path = tmpdir.join(
            'db-path',
            'last-frame/fails.trace').strpath
        self.gl_replay_crashes_trace_path = tmpdir.join(
            'db-path',
            'replay/fails.trace').strpath
        self.gl_trace_calls = '1211,1384'
        self.gl_trace_last_call = '1413'
        self.gl_trace_wrong_call = '1414'
        self.output_dir = tmpdir.mkdir('results').strpath
        self.dxgi_trace_path = tmpdir.join(
            'db-path',
            'Wicked-Engine/Tests:Cloth_Physics_Test.trace-dxgi').strpath
        self.dxgi_last_frame_fails_trace_path = tmpdir.join(
            'db-path',
            'last-frame/fails.trace-dxgi').strpath
        self.dxgi_replay_crashes_trace_path = tmpdir.join(
            'db-path',
            'replay/fails.trace-dxgi').strpath
        self.dxgi_trace_calls = '235747,257964'
        self.dxgi_trace_last_call = '273345'
        self.dxgi_trace_wrong_call = '273346'
        self.results_partial_path = path.join('trace', OPTIONS.device_name)
        self.m_apitrace_subprocess_run = mocker.patch(
            'framework.replay.backends.apitrace.subprocess.run',
            side_effect=self.mock_apitrace_subprocess_run)
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
        test = backends.apitrace.APITraceBackend('unsupported_trace.gfxr')

    @pytest.mark.parametrize('option, apitrace, eglretrace', [
        (0, '/env/apitrace', '/env/eglretrace'),
        (1, '/config/apitrace', '/config/eglretrace'),
        (2, 'apitrace', 'eglretrace'),
    ])
    def test_dump_gl_options(self, option, apitrace, eglretrace, config):
        """Tests for the dump method: basic with options.

        Check basic GL dumps with different configurations. No specific output
        directory is provided and leaving to the method to figure out the last
        call from the trace file itself.

        """
        self.apitrace = apitrace
        self.eglretrace = eglretrace
        if option == 0:
            self.env['PIGLIT_REPLAY_APITRACE_BINARY'] = self.apitrace
            self.env['PIGLIT_REPLAY_EGLRETRACE_BINARY'] = self.eglretrace
        elif option == 1:
            config.set('replay', 'apitrace_bin', self.apitrace)
            config.set('replay', 'eglretrace_bin', self.eglretrace)
        calls = self.gl_trace_last_call
        trace_path = self.gl_trace_path
        test = backends.apitrace.APITraceBackend(trace_path)
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        m_calls = [self.mocker.call(
            [self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.eglretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_gl_output(self):
        """Tests for the dump method: explicit output directory.

        Check a basic GL dump, specifying the output and leaving for the method
        to figure out the last call from the trace file itself.

        """
        calls = self.gl_trace_last_call
        trace_path = self.gl_trace_path
        test = backends.apitrace.APITraceBackend(trace_path,
                                                 output_dir=self.output_dir)
        assert test.dump()
        snapshot_prefix = path.join(self.output_dir,
                                    path.basename(trace_path) + '-')
        m_calls = [self.mocker.call(
            [self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.eglretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_gl_calls(self):
        """Tests for the dump method: explicit valid calls.

        Check a basic GL dump, specifying valid calls to dump. No specific
        output directory is provided.

        """
        calls = self.gl_trace_calls
        trace_path = self.gl_trace_path
        test = backends.apitrace.APITraceBackend(trace_path,
                                                 calls=calls.split(','))
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_apitrace_subprocess_run.assert_called_once_with(
            [self.eglretrace, '--headless',
             '--snapshot=' + calls,
             '--snapshot-prefix=' + snapshot_prefix, trace_path],
            env=None, stdout=subprocess.PIPE)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_gl_wrong_call(self):
        """Tests for the dump method: explicit invalid call.

        Check a basic GL dump, specifying an invalid call to dump. No specific
        output directory is provided.

        """
        calls = self.gl_trace_wrong_call
        trace_path = self.gl_trace_path
        test = backends.apitrace.APITraceBackend(trace_path,
                                                 calls=calls.split(','))
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_apitrace_subprocess_run.assert_called_once_with(
            [self.eglretrace, '--headless',
             '--snapshot=' + calls,
             '--snapshot-prefix=' + snapshot_prefix, trace_path],
            env=None, stdout=subprocess.PIPE)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_gl_last_frame_fails(self):
        """Tests for the dump method: the call to figure out the last frame fails.

        Check a basic GL dump. The call to figure out the last frame fails.

        """
        calls = '-1'
        trace_path = self.gl_last_frame_fails_trace_path
        test = backends.apitrace.APITraceBackend(trace_path)
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        m_calls = [self.mocker.call(
            [self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.eglretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_gl_replay_crashes(self):
        """Tests for the dump method: the replay call crashes.

        Check a basic GL dump. The replay call crashes.

        """
        calls = self.gl_trace_last_call
        trace_path = self.gl_replay_crashes_trace_path
        test = backends.apitrace.APITraceBackend(trace_path)
        assert not test.dump()
        snapshot_prefix = trace_path + '-'
        m_calls = [self.mocker.call(
            [self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.eglretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call.zfill(10) + '.png')

    @pytest.mark.parametrize('option, wine, apitrace, d3dretrace', [
        (0, '/env/wine', '/env/wine/apitrace', '/env/wine/d3dretrace'),
        (1, '/config/wine', '/config/wine/apitrace', '/config/wine/d3dretrace'),
        (2, 'wine', 'apitrace', 'd3dretrace'),
    ])
    def test_dump_dxgi_options(self, option, wine, apitrace, d3dretrace, config):
        """Tests for the dump method: basic with options.

        Check basic DXGI dumps with different configurations. No specific output
        directory is provided and leaving to the method to figure out the last
        call from the trace file itself.

        """
        self.wine = wine
        self.apitrace = apitrace
        self.d3dretrace = d3dretrace
        if option == 0:
            self.env['PIGLIT_REPLAY_WINE_BINARY'] = self.wine
            self.env['PIGLIT_REPLAY_WINE_APITRACE_BINARY'] = self.apitrace
            self.env['PIGLIT_REPLAY_WINE_D3DRETRACE_BINARY'] = self.d3dretrace
        elif option == 1:
            config.set('replay', 'wine_bin', self.wine)
            config.set('replay', 'wine_apitrace_bin', self.apitrace)
            config.set('replay', 'wine_d3dretrace_bin', self.d3dretrace)
        calls = self.dxgi_trace_last_call
        trace_path = self.dxgi_trace_path
        test = backends.apitrace.APITraceBackend(trace_path)
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        m_calls = [self.mocker.call(
            [self.wine, self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.wine, self.d3dretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_dxgi_output(self):
        """Tests for the dump method: explicit output directory.

        Check a basic DXGI dump, specifying the output and leaving for the method
        to figure out the last call from the trace file itself.

        """
        calls = self.dxgi_trace_last_call
        trace_path = self.dxgi_trace_path
        test = backends.apitrace.APITraceBackend(trace_path,
                                                 output_dir=self.output_dir)
        assert test.dump()
        snapshot_prefix = path.join(self.output_dir,
                                    path.basename(trace_path) + '-')
        m_calls = [self.mocker.call(
            [self.wine, self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.wine, self.d3dretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_dxgi_calls(self):
        """Tests for the dump method: explicit valid calls.

        Check a basic DXGI dump, specifying valid calls to dump. No specific
        output directory is provided.

        """
        calls = self.dxgi_trace_calls
        trace_path = self.dxgi_trace_path
        test = backends.apitrace.APITraceBackend(trace_path,
                                                 calls=calls.split(','))
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_apitrace_subprocess_run.assert_called_once_with(
            [self.wine, self.d3dretrace, '--headless',
             '--snapshot=' + calls,
             '--snapshot-prefix=' + snapshot_prefix, trace_path],
            env=None, stdout=subprocess.PIPE)
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_dxgi_wrong_call(self):
        """Tests for the dump method: explicit invalid call.

        Check a basic DXGI dump, specifying an invalid call to dump. No
        specific output directory is provided.

        """
        calls = self.dxgi_trace_wrong_call
        trace_path = self.dxgi_trace_path
        test = backends.apitrace.APITraceBackend(trace_path,
                                                 calls=calls.split(','))
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_apitrace_subprocess_run.assert_called_once_with(
            [self.wine, self.d3dretrace, '--headless',
             '--snapshot=' + calls,
             '--snapshot-prefix=' + snapshot_prefix, trace_path],
            env=None, stdout=subprocess.PIPE)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_dxgi_last_frame_fails(self):
        """Tests for the dump method: the call to figure out the last frame fails.

        Check a basic DXGI dump. The call to figure out the last frame fails.

        """
        calls = '-1'
        trace_path = self.dxgi_last_frame_fails_trace_path
        test = backends.apitrace.APITraceBackend(trace_path)
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        m_calls = [self.mocker.call(
            [self.wine, self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.wine, self.d3dretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call.zfill(10) + '.png')

    def test_dump_dxgi_replay_crashes(self):
        """Tests for the dump method: the replay call crashes.

        Check a basic DXGI dump. The replay call crashes.

        """
        calls = self.dxgi_trace_last_call
        trace_path = self.dxgi_replay_crashes_trace_path
        test = backends.apitrace.APITraceBackend(trace_path)
        assert not test.dump()
        snapshot_prefix = trace_path + '-'
        m_calls = [self.mocker.call(
            [self.wine, self.apitrace, 'dump', '--calls=frame', trace_path],
            stdout=subprocess.PIPE),
                   self.mocker.call(
                       [self.wine, self.d3dretrace, '--headless',
                        '--snapshot=' + calls,
                        '--snapshot-prefix=' + snapshot_prefix, trace_path],
                       env=None, stdout=subprocess.PIPE)]
        assert self.m_apitrace_subprocess_run.call_count == 2
        self.m_apitrace_subprocess_run.assert_has_calls(m_calls)
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call.zfill(10) + '.png')
