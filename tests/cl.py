#!/usr/bin/env python
# -*- coding: utf-8 -*-
"""All OpenCL tests that come with piglit, using default settings."""

# Because these profiles put all of the names in the global scope and rely on
# __all__ to hide private names there will be a lot errors from pylint about
# invalid constant names, they're not really fixable, so just hide them.
# pylint: disable=invalid-name

from __future__ import division, absolute_import, print_function

import os

from framework.profile import TestProfile
from framework.test import PiglitCLTest
from framework import grouptools
from .py_modules.constants import TESTS_DIR, GENERATED_TESTS_DIR

__all__ = ['profile']

profile = TestProfile()

# Custom
with profile.group_manager(PiglitCLTest, 'custom') as g:
    g(['cl-custom-run-simple-kernel'], 'Run simple kernel')
    g(['cl-custom-flush-after-enqueue-kernel'], 'Flush after enqueue kernel')
    g(['cl-custom-r600-create-release-buffer-bug'],
      'r600 create release buffer bug')
    g(['cl-custom-buffer-flags'], 'Buffer flags')

with profile.group_manager(PiglitCLTest, 'api') as g:
    # Platform
    g(['cl-api-get-platform-ids'], 'clGetPlatformIDs')
    g(['cl-api-get-platform-info'], 'clGetPlatformInfo')

    # Device
    g(['cl-api-get-device-ids'], 'clGetDeviceIDs')
    g(['cl-api-get-device-info'], 'clGetDeviceInfo')

    # Context
    g(['cl-api-create-context'], 'clCreateContext')
    g(['cl-api-create-context-from-type'], 'clCreateContextFromType')
    g(['cl-api-get-context-info'], 'clGetContextInfo')
    g(['cl-api-retain_release-context'],
      'clRetainContext and clReleaseContext')

    # Command Queues
    g(['cl-api-create-command-queue'], 'clCreateCommandQueue')
    g(['cl-api-retain_release-command-queue'],
      'clRetainComandQueue and clReleaseCommandQueue')
    g(['cl-api-get-command-queue-info'], 'clGetCommandQueueInfo')

    # Memory objects
    g(['cl-api-create-buffer'], 'clCreateBuffer')
    g(['cl-api-create-image'], 'clCreateImage')
    g(['cl-api-create-sampler'], 'clCreateSampler')
    g(['cl-api-enqueue-copy-buffer'], 'clEnqueueCopyBuffer')
    g(['cl-api-enqueue-copy-buffer-rect'], 'clEnqueueCopyBufferRect')
    g(['cl-api-enqueue-read_write-buffer'],
      'clEnqueueReadBuffer and clEnqueueWriteBuffer')
    g(['cl-api-get-mem-object-info'], 'clGetMemObjectInfo')
    g(['cl-api-get-image-info'], 'clGetImageInfo')
    g(['cl-api-retain_release-mem-object'],
      'clRetainMemObject and clReleaseMemObject')

    g(['cl-api-create-program-with-binary'], 'clCreateProgramWithBinary')
    g(['cl-api-create-program-with-source'], 'clCreateProgramWithSource')
    g(['cl-api-build-program'], 'clBuildProgram')
    g(['cl-api-compile-program'], 'clCompileProgram')
    g(['cl-api-get-program-info'], 'clGetProgramInfo')
    g(['cl-api-get-program-build-info'], 'clGetProgramBuildInfo')
    g(['cl-api-retain_release-program'],
      'clRetainProgram and clReleaseProgram')
    g(['cl-api-unload-compiler'], 'clUnloadCompiler')

    # Kernel
    g(['cl-api-create-kernel'], 'clCreateKernel')
    g(['cl-api-create-kernels-in-program'], 'clCreateKernelsInProgram')
    g(['cl-api-get-kernel-info'], 'clGetKernelInfo')
    g(['cl-api-get-kernel-work-group-info'], 'clGetKernelWorkGroupInfo')
    g(['cl-api-retain_release-kernel'], 'clRetainKernel and clReleaseKernel')
    g(['cl-api-set-kernel-arg'], 'clSetKernelArg')

    # Event
    g(['cl-api-get-event-info'], 'clGetEventInfo')
    g(['cl-api-retain_release-event'], 'clRetainEvent and clReleaseEvent')

with profile.group_manager(PiglitCLTest, 'program') as g:
    g(['cl-program-max-work-item-sizes'],
      'Run kernel with max work item sizes')
    g(['cl-program-bitcoin-phatk'], 'Bitcoin: phatk kernel')

with profile.group_manager(PiglitCLTest, 'interop') as g:
    g(['cl-interop-egl_khr_cl_event2'], 'EGL_KHR_cl_event2')


def add_program_test_dir(group, dirpath):
    for filename in os.listdir(dirpath):
        testname, ext = os.path.splitext(filename)
        if ext not in ['.cl', '.program_test']:
            continue

        profile.test_list[grouptools.join(group, testname)] = PiglitCLTest(
            ['cl-program-tester', os.path.join(dirpath, filename)])


add_program_test_dir(grouptools.join('program', 'build'),
                     os.path.join(TESTS_DIR, 'cl', 'program', 'build'))
add_program_test_dir(grouptools.join('program', 'build', 'fail'),
                     os.path.join(TESTS_DIR, 'cl', 'program', 'build', 'fail'))
add_program_test_dir(grouptools.join('program', 'execute'),
                     os.path.join(TESTS_DIR, 'cl', 'program', 'execute'))
add_program_test_dir(grouptools.join('program', 'execute'),
                     os.path.join(TESTS_DIR, 'cl', 'program', 'execute',
                                  'builtin', 'atomic'))
add_program_test_dir(grouptools.join('program', 'execute'),
                     os.path.join(TESTS_DIR, 'cl', 'program', 'execute',
                                  'builtin', 'convert'))

# Run generated built-in tests
add_program_test_dir(grouptools.join('program', 'execute', 'builtin'),
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'builtin', 'int'))
add_program_test_dir(grouptools.join('program', 'execute', 'builtin'),
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'builtin',
                                  'math'))
add_program_test_dir(grouptools.join('program', 'execute', 'builtin'),
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'builtin',
                                  'relational'))
add_program_test_dir(grouptools.join('program', 'execute', 'builtin'),
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'builtin',
                                  'common'))
add_program_test_dir(grouptools.join('program', 'execute', 'store'),
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'store'))
