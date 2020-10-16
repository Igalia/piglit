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


"""Tests for replayer's compare_replay module."""

import pytest

import contextlib
import io
import os

from os import path

from framework import exceptions
from framework.replay import backends
from framework.replay import compare_replay
from framework.replay.options import OPTIONS


class TestCompareReplay(object):
    """Tests for compare_replay methods."""

    @staticmethod
    def _create_dump(trace_path, results_path, calls):
        p = path.join(results_path,
                      path.basename(trace_path) + '-' + str(calls) + '.png')
        os.makedirs(path.dirname(p), exist_ok=True)
        with open(p, 'w') as f:
            f.write('content')

    @staticmethod
    def mock_backends_dump(trace_path, results_path, calls):
        if trace_path.endswith('KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr'):
            TestCompareReplay._create_dump(trace_path, results_path, 99)

            return True
        elif trace_path.endswith('pathfinder/demo.trace'):
            TestCompareReplay._create_dump(trace_path, results_path, 78)

            return True
        elif trace_path.endswith('Wicked-Engine/Tests:Cloth_Physics_Test.trace-dxgi'):
            return False
        elif trace_path.endswith('unimplemented/backend.vktrace'):
            raise backends.DumpBackendNotImplementedError(
                'DumpBackend for "vktrace" is not implemented')
        elif trace_path.endswith('unexisting/back.end'):
            raise backends.DumpBackendError(
                'No module supports file extensions "end"')
        else:
            raise exceptions.PiglitFatalError(
                'Non treated trace path: {}'.format(trace_path))

    @staticmethod
    def mock_hexdigest_from_image(image_file):
        if image_file.endswith(
                'KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr-99.png'):
            return '917cbbf4f09dd62ea26d247a1c70c16e'
        elif image_file.endswith(
                'pathfinder/demo.trace-78.png'):
            return 'e624d76c70cc3c532f4f54439e13659a'
        else:
            raise exceptions.PiglitFatalError(
                'Non treated image file: {}'.format(image_file))

    @staticmethod
    def mock_qty_load_yaml(yaml_file):
        if yaml_file == 'empty.yml':
            return {}
        elif yaml_file == 'no-device.yml':
            return {"traces":
              [{"path": "glmark2/desktop-blur-radius=5:effect=blur:passes=1:separable=true:windows=4.rdc",
                "expectations": [{"device": "gl-vmware-llvmpipe",
                                  "checksum": "8867f3a41f180626d0d4b7661ff5c0f4"}]},]}
        elif yaml_file == 'one-trace.yml':
            return {"traces":
              [{"path": "KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr",
                "expectations": [{"device": OPTIONS.device_name,
                                  "checksum": "917cbbf4f09dd62ea26d247a1c70c16e"}]},]}
        elif yaml_file == 'two-traces.yml':
            return {"traces":
              [{"path": "pathfinder/demo.trace",
                "expectations": [{"device": OPTIONS.device_name,
                                  "checksum": "e624d76c70cc3c532f4f54439e13659a"}]},
               {"path": "KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr",
                "expectations": [{"device": OPTIONS.device_name,
                                  "checksum": "917cbbf4f09dd62ea26d247a1c70c16e"}]},]}
        else:
            raise exceptions.PiglitFatalError(
                'Non treated YAML file: {}'.format(yaml_file))

    @pytest.fixture(autouse=True)
    def setup(self, mocker, tmpdir):
        """Setup for TestCompareReplay.

        This create the basic environment for testing.
        """

        OPTIONS.device_name = 'test-device'
        OPTIONS.db_path = tmpdir.mkdir('db-path').strpath
        OPTIONS.results_path = tmpdir.mkdir('results').strpath
        self.trace_path = 'KhronosGroup-Vulkan-Tools/amd/polaris10/vkcube.gfxr'
        self.exp_checksum = '917cbbf4f09dd62ea26d247a1c70c16e'
        self.results_partial_path = path.join('results/trace',
                                              OPTIONS.device_name)
        self.m_qty_load_yaml = mocker.patch(
            'framework.replay.compare_replay.qty.load_yaml',
            side_effect=TestCompareReplay.mock_qty_load_yaml)
        self.m_ensure_file = mocker.patch(
            'framework.replay.compare_replay.ensure_file',
            return_value=None)
        self.m_backends_dump = mocker.patch(
            'framework.replay.compare_replay.backends.dump',
            side_effect=TestCompareReplay.mock_backends_dump)
        self.m_hexdigest_from_image = mocker.patch(
            'framework.replay.compare_replay.hexdigest_from_image',
            side_effect=TestCompareReplay.mock_hexdigest_from_image)
        self.tmpdir = tmpdir

    def test_from_yaml_empty(self):
        """compare_replay.from_yaml: compare using an empty YAML file"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.from_yaml('empty.yml')
                    is compare_replay.Result.MATCH)
        self.m_qty_load_yaml.assert_called_once()
        s = f.getvalue()
        assert s == ''

    def test_from_yaml_no_device(self):
        """compare_replay.from_yaml: compare using a YAML without expectations for the used device"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.from_yaml('no-device.yml')
                    is compare_replay.Result.MATCH)
        self.m_qty_load_yaml.assert_called_once()
        s = f.getvalue()
        assert s == ''

    def test_from_yaml_one_trace(self):
        """compare_replay.from_yaml: compare using a YAML with just one expectation for the used device"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.from_yaml('one-trace.yml')
                    is compare_replay.Result.MATCH)
        self.m_qty_load_yaml.assert_called_once()
        self.m_ensure_file.assert_called_once()
        self.m_backends_dump.assert_called_once()
        self.m_hexdigest_from_image.assert_called_once()
        assert not self.tmpdir.join(self.results_partial_path,
                                    self.trace_path + '-99.png').check()
        assert not self.tmpdir.join(self.results_partial_path,
                                    path.dirname(self.trace_path),
                                    self.exp_checksum + '.png').check()
        s = f.getvalue()
        assert s == ('[check_image]\n'
                     '    actual: ' + self.exp_checksum + '\n'
                     '  expected: ' + self.exp_checksum + '\n'
                     '[check_image] Images match for:\n'
                     '  ' + self.trace_path + '\n'
                     '\n')

    def test_from_yaml_two_traces(self):
        """compare_replay.from_yaml: compare using a YAML with more than one expectation for the used device"""

        second_trace_path = 'pathfinder/demo.trace'
        second_exp_checksum = 'e624d76c70cc3c532f4f54439e13659a'
        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.from_yaml('two-traces.yml')
                    is compare_replay.Result.MATCH)
        self.m_qty_load_yaml.assert_called_once()
        assert self.m_ensure_file.call_count == 2
        assert self.m_backends_dump.call_count == 2
        assert self.m_hexdigest_from_image.call_count == 2
        assert not self.tmpdir.join(self.results_partial_path,
                                    second_trace_path + '-78.png').check()
        assert not self.tmpdir.join(self.results_partial_path,
                                    path.dirname(second_trace_path),
                                    second_exp_checksum + '.png').check()
        assert not self.tmpdir.join(self.results_partial_path,
                                    self.trace_path + '-99.png').check()
        assert not self.tmpdir.join(self.results_partial_path,
                                    path.dirname(self.trace_path),
                                    self.exp_checksum + '.png').check()
        s = f.getvalue()
        assert s == ('[check_image]\n'
                     '    actual: ' + second_exp_checksum + '\n'
                     '  expected: ' + second_exp_checksum + '\n'
                     '[check_image] Images match for:\n'
                     '  ' + second_trace_path + '\n'
                     '\n'
                     '[check_image]\n'
                     '    actual: ' + self.exp_checksum + '\n'
                     '  expected: ' + self.exp_checksum + '\n'
                     '[check_image] Images match for:\n'
                     '  ' + self.trace_path + '\n'
                     '\n')

    def test_trace_success(self):
        """compare_replay.trace: compare a trace successfully"""

        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.trace(self.trace_path, self.exp_checksum)
                    is compare_replay.Result.MATCH)
        self.m_qty_load_yaml.assert_not_called()
        self.m_ensure_file.assert_called_once()
        self.m_backends_dump.assert_called_once()
        self.m_hexdigest_from_image.assert_called_once()
        assert not self.tmpdir.join(self.results_partial_path,
                                    self.trace_path + '-99.png').check()
        assert not self.tmpdir.join(self.results_partial_path,
                                    path.dirname(self.trace_path),
                                    self.exp_checksum + '.png').check()
        s = f.getvalue()
        assert s.endswith('PIGLIT: {"result": "pass"}\n')

    def test_trace_success_keep_image(self):
        """compare_replay.trace: compare a trace successfully and set the option to keep the dumped image"""

        OPTIONS.keep_image = True
        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.trace(self.trace_path, self.exp_checksum)
                    is compare_replay.Result.MATCH)
        self.m_qty_load_yaml.assert_not_called()
        self.m_ensure_file.assert_called_once()
        self.m_backends_dump.assert_called_once()
        self.m_hexdigest_from_image.assert_called_once()
        assert not self.tmpdir.join(self.results_partial_path,
                                    self.trace_path + '-99.png').check()
        assert self.tmpdir.join(self.results_partial_path,
                                path.dirname(self.trace_path),
                                self.exp_checksum + '.png').check()
        s = f.getvalue()
        assert s.endswith('PIGLIT: {"result": "pass"}\n')

    def test_trace_fail(self):
        """compare_replay.trace: fail comparing a trace"""

        wrong_checksum = '917cbbf4f09dd62ea26d247a1c70c16f'
        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.trace(self.trace_path, wrong_checksum)
                    is compare_replay.Result.DIFFER)
        self.m_qty_load_yaml.assert_not_called()
        self.m_ensure_file.assert_called_once()
        self.m_backends_dump.assert_called_once()
        self.m_hexdigest_from_image.assert_called_once()
        assert not self.tmpdir.join(self.results_partial_path,
                                    self.trace_path + '-99.png').check()
        assert self.tmpdir.join(self.results_partial_path,
                                path.dirname(self.trace_path),
                                self.exp_checksum + '.png').check()
        s = f.getvalue()
        assert s.endswith('PIGLIT: '
                          '{"images": [{'
                          '"image_desc": "' + self.trace_path + '", '
                          '"image_ref": "' + wrong_checksum + '.png", '
                          '"image_render": "' +
                          self.tmpdir.join(self.results_partial_path,
                                           path.dirname(self.trace_path),
                                           self.exp_checksum +
                                           '.png').strpath +
                          '"}], "result": "fail"}\n')

    @pytest.mark.parametrize('trace_path', [
        ('Wicked-Engine/Tests:Cloth_Physics_Test.trace-dxgi'),
        ('unimplemented/backend.vktrace'),
        ('unexisting/back.end'),
    ])
    def test_trace_dump_crash(self, trace_path):
        """compare_replay.trace: dump crashes or fails comparing a trace"""

        third_exp_checksum = '6b6d27df609b8d086cc3335e6d103581'
        f = io.StringIO()
        with contextlib.redirect_stdout(f):
            assert (compare_replay.trace(trace_path, third_exp_checksum)
                    is compare_replay.Result.FAILURE)
        self.m_qty_load_yaml.assert_not_called()
        self.m_ensure_file.assert_called_once()
        self.m_backends_dump.assert_called_once()
        self.m_hexdigest_from_image.assert_not_called()
        assert not self.tmpdir.join(self.results_partial_path,
                                    path.dirname(trace_path),
                                    third_exp_checksum + '.png').check()
        s = f.getvalue()
        assert s.endswith('PIGLIT: {"result": "crash"}\n')
