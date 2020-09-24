# coding=utf-8
#
# Copyright (c) 2014, 2016, 2019 Intel Corporation
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


""" Base class for dump backends

This module provides a base class for replayer dump backend modules.

"""

import abc
import functools
import subprocess

from os import path

from framework import core
from framework.replay.options import OPTIONS


class DumpBackend(metaclass=abc.ABCMeta):
    """ Base class for dump backends

    This class provides an basic ancestor for classes implementing dump
    backends, providing a light public API. The goal of this API is to be "just
    enough", not a generic writing solution. To that end it provides the
    method, 'dump'. This method is designed to be just enough to write a
    backend without needing format specific options.

    """
    def __init__(self, trace_path, output_dir=None, calls=[], **kwargs):
        """ Generic constructor

        This method takes keyword arguments that define options for the
        backends. Options should be prefixed to identify which backends they
        apply to. For example, an apitrace specific value should be passed as
        apitrace_*, while a file gfxrecon value should be passed as gfxrecon_*)

        Arguments:

        trace_path -- the path to the trace from which we want to dump calls as
                      images.
        output_dir -- the place to write the images to.
        calls      -- an array of the calls in the trace for which we want to
                      dump images.

        """
        self._trace_path = trace_path
        self._output_dir = output_dir
        self._calls = calls

        if self._output_dir is None:
            self._output_dir = path.join('trace', OPTIONS.device_name,
                                         path.dirname(self._trace_path))


    @staticmethod
    def log(severity, msg, end='\n'):
        print('[dump_trace_images] {}: {}'.format(severity, msg), flush=True,
              end=end)


    @staticmethod
    def log_result(msg):
        print(msg, flush=True)


    @staticmethod
    def _run_logged_command(cmd, env):
        ret = subprocess.run(cmd, stdout=subprocess.PIPE, env=env)
        logoutput = '[dump_trace_images] Running: {}\n'.format(
            ' '.join(cmd)).encode() + ret.stdout
        print(logoutput.decode(errors='replace'))
        if ret.returncode:
            raise RuntimeError(
                '[dump_traces_images] Process failed with error code: {}'.format(
                    ret.returncode))


    @abc.abstractmethod
    def _get_last_frame_call(self):
        """Get the number of the last frame call from the trace"""

    def dump(self):
        """ Dump the calls to images from the trace

        This method actually dumps the calls from a trace.

        """
        self.log('Info', 'Dumping trace {}'.format(self._trace_path),
                 end='...\n')
        core.check_dir(self._output_dir)


def dump_handler(func):
    """ Decorator function for handling trace dumps.

    This will handle exceptions and log the result.

    """

    @functools.wraps(func)
    def _inner(*args, **kwargs):
        try:
            func(*args, **kwargs)
            DumpBackend.log_result('OK')
            return True
        except Exception as e:
            DumpBackend.log_result('ERROR')
            DumpBackend.log('Debug', '=== Failure log start ===')
            print(e)
            DumpBackend.log('Debug', '=== Failure log end ===')
            return False

    return _inner
