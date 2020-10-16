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


"""Tests for replayer's renderdoc backend."""

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

class TestRenderDocBackend(object):
    """Tests for the RenderDocBackend class."""

    def mock_renderdoc_subprocess_run(self, cmd, stdout, env=None):
        ret = subprocess.CompletedProcess(cmd, 0)
        if len(cmd) > 3:
            calls = cmd[3:]
        else:
            calls = [self.gl_trace_last_call]
        if (cmd[1] == self.gl_trace_path and
            calls != [self.gl_trace_wrong_call]):
            prefix = path.join(cmd[2], path.basename(cmd[1]))
            ret.stdout = b''
            for call in calls:
                if call != self.gl_trace_wrong_call:
                    dump = prefix + '-' + call + '.png'
                    with open(dump, 'w') as f:
                        f.write("content")
                    if call == self.gl_trace_last_call:
                        call_text = 'End of Capture'
                    else:
                        call_text = 'glDrawArrays(4)'
                    ret.stdout += bytearray('Saving image at eventId ' + call +
                                            ': ' + call_text + ' to ' + dump +
                                            '\n', 'utf-8')
        else:
            ret.stdout = b''
            ret.returncode = 1

        return ret

    @pytest.fixture(autouse=True)
    def setup(self, mocker, tmpdir):
        """Setup for TestRenderDocBackend.

        This set ups the basic environment for testing.
        """

        OPTIONS.device_name = 'test-device'
        self.renderdoc = 'renderdoc/renderdoc_dump_images.py'
        self.gl_trace_path = tmpdir.mkdir(
            'db-path').join('glmark2/desktop.rdc').strpath
        self.gl_replay_crashes_trace_path = tmpdir.join(
            'db-path',
            'replay/fails.rdc').strpath
        self.gl_trace_calls = '47,332'
        self.gl_trace_last_call = '340'
        self.gl_trace_wrong_call = '333'
        self.output_dir = tmpdir.mkdir('results').strpath
        self.results_partial_path = path.join('trace', OPTIONS.device_name)
        self.m_renderdoc_subprocess_run = mocker.patch(
            'framework.replay.backends.abstract.subprocess.run',
            side_effect=self.mock_renderdoc_subprocess_run)
        self.tmpdir = tmpdir
        self.mocker = mocker

    @pytest.mark.raises(exception=exceptions.PiglitFatalError)
    def test_init_unsupported_trace(self):
        """Tests for the init method.

        Should raise an exception in case of creating with an unsupported trace
        format.

        """
        test = backends.renderdoc.RenderDocBackend('unsupported_trace.gfxr')

    def test_dump_gl_options(self):
        """Tests for the dump method: basic with options.

        Check basic GL dumps with different configurations. No specific output
        directory is provided and leaving to the method to figure out the last
        call from the trace file itself.

        """
        calls = self.gl_trace_last_call
        trace_path = self.gl_trace_path
        test = backends.renderdoc.RenderDocBackend(trace_path)
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_renderdoc_subprocess_run.assert_called_once()
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call + '.png')

    def test_dump_gl_output(self):
        """Tests for the dump method: explicit output directory.

        Check a basic GL dump, specifying the output and leaving for the method
        to figure out the last call from the trace file itself.

        """
        calls = self.gl_trace_last_call
        trace_path = self.gl_trace_path
        test = backends.renderdoc.RenderDocBackend(trace_path,
                                                   output_dir=self.output_dir)
        assert test.dump()
        snapshot_prefix = path.join(self.output_dir,
                                    path.basename(trace_path) + '-')
        self.m_renderdoc_subprocess_run.assert_called_once()
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call + '.png')

    def test_dump_gl_calls(self):
        """Tests for the dump method: explicit valid calls.

        Check a basic GL dump, specifying valid calls to dump. No specific
        output directory is provided.

        """
        calls = self.gl_trace_calls
        trace_path = self.gl_trace_path
        test = backends.renderdoc.RenderDocBackend(trace_path,
                                                   calls=calls.split(','))
        assert test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_renderdoc_subprocess_run.assert_called_once()
        for call in calls.split(','):
            assert path.exists(snapshot_prefix + call + '.png')

    def test_dump_gl_wrong_call(self):
        """Tests for the dump method: explicit invalid call.

        Check a basic GL dump, specifying an invalid call to dump. No specific
        output directory is provided.

        """
        calls = self.gl_trace_wrong_call
        trace_path = self.gl_trace_path
        test = backends.renderdoc.RenderDocBackend(trace_path,
                                                   calls=calls.split(','))
        assert not test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_renderdoc_subprocess_run.assert_called_once()
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call + '.png')

    def test_dump_gl_replay_crashes(self):
        """Tests for the dump method: the replay call crashes.

        Check a basic GL dump. The replay call crashes.

        """
        calls = self.gl_trace_last_call
        trace_path = self.gl_replay_crashes_trace_path
        test = backends.renderdoc.RenderDocBackend(trace_path)
        assert not test.dump()
        snapshot_prefix = trace_path + '-'
        self.m_renderdoc_subprocess_run.assert_called_once()
        for call in calls.split(','):
            assert not path.exists(snapshot_prefix + call + '.png')
