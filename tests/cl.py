#!/usr/bin/env python
# -*- coding: utf-8 -*-
# All OpenCL tests that come with piglit, using default settings

from __future__ import division, absolute_import, print_function

import os
import os.path as path
import platform
import glob

from framework.profile import TestProfile
from framework.test import PiglitCLTest
from .py_modules.constants import TESTS_DIR, GENERATED_TESTS_DIR

__all__ = ['profile']

######
# Helper functions

can_do_concurrent = platform.system().lower()[0:5] != 'linux' or glob.glob('/dev/dri/render*')

def add_plain_test(group, name, args):
        group[name] = PiglitCLTest(args, run_concurrent=can_do_concurrent)

def add_plain_program_tester_test(group, name, path):
        add_plain_test(group, name, ['cl-program-tester', path])

######
# Collecting all tests
profile = TestProfile()

custom = {}
api = {}
program = {}
profile.tests['Custom'] = custom
profile.tests['API'] = api
profile.tests['Program'] = program

######
# Tests

# Custom
add_plain_test(custom, 'Run simple kernel', ['cl-custom-run-simple-kernel'])
add_plain_test(custom, 'Flush after enqueue kernel', ['cl-custom-flush-after-enqueue-kernel'])
add_plain_test(custom, 'r600 create release buffer bug', ['cl-custom-r600-create-release-buffer-bug'])
add_plain_test(custom, 'Buffer flags', ['cl-custom-buffer-flags'])

# API
#  Platform
add_plain_test(api, 'clGetPlatformIDs', ['cl-api-get-platform-ids'])
add_plain_test(api, 'clGetPlatformInfo', ['cl-api-get-platform-info'])
#  Device
add_plain_test(api, 'clGetDeviceIDs', ['cl-api-get-device-ids'])
add_plain_test(api, 'clGetDeviceInfo', ['cl-api-get-device-info'])
#  Context
add_plain_test(api, 'clCreateContext', ['cl-api-create-context'])
add_plain_test(api, 'clCreateContextFromType', ['cl-api-create-context-from-type'])
add_plain_test(api, 'clGetContextInfo', ['cl-api-get-context-info'])
add_plain_test(api, 'clRetainContext and clReleaseContext', ['cl-api-retain_release-context'])
#  Command Queues
add_plain_test(api, 'clCreateCommandQueue', ['cl-api-create-command-queue'])
add_plain_test(api, 'clRetainComandQueue and clReleaseCommandQueue', ['cl-api-retain_release-command-queue'])
add_plain_test(api, 'clGetCommandQueueInfo', ['cl-api-get-command-queue-info'])
#  Memory objects
add_plain_test(api, 'clCreateBuffer', ['cl-api-create-buffer'])
add_plain_test(api, 'clCreateImage', ['cl-api-create-image'])
add_plain_test(api, 'clCreateSampler', ['cl-api-create-sampler'])
add_plain_test(api, 'clEnqueueCopyBuffer', ['cl-api-enqueue-copy-buffer'])
add_plain_test(api, 'clEnqueueCopyBufferRect', ['cl-api-enqueue-copy-buffer-rect'])
add_plain_test(api, 'clEnqueueReadBuffer and clEnqueueWriteBuffer', ['cl-api-enqueue-read_write-buffer'])
add_plain_test(api, 'clGetMemObjectInfo', ['cl-api-get-mem-object-info'])
add_plain_test(api, 'clGetImageInfo', ['cl-api-get-image-info'])
add_plain_test(api, 'clRetainMemObject and clReleaseMemObject', ['cl-api-retain_release-mem-object'])
#  Program
add_plain_test(api, 'clCreateProgramWithBinary', ['cl-api-create-program-with-binary'])
add_plain_test(api, 'clCreateProgramWithSource', ['cl-api-create-program-with-source'])
add_plain_test(api, 'clBuildProgram', ['cl-api-build-program'])
add_plain_test(api, 'clCompileProgram', ['cl-api-compile-program'])
add_plain_test(api, 'clCreateKernelsInProgram', ['cl-api-create-kernels-in-program'])
add_plain_test(api, 'clGetProgramInfo', ['cl-api-get-program-info'])
add_plain_test(api, 'clGetProgramBuildInfo', ['cl-api-get-program-build-info'])
add_plain_test(api, 'clRetainProgram and clReleaseProgram', ['cl-api-retain_release-program'])
add_plain_test(api, 'clUnloadCompiler', ['cl-api-unload-compiler'])
#  Kernel
add_plain_test(api, 'clCreateKernel', ['cl-api-create-kernel'])
add_plain_test(api, 'clCreateKernelsInProgram', ['cl-api-create-kernels-in-program'])
add_plain_test(api, 'clGetKernelInfo', ['cl-api-get-kernel-info'])
add_plain_test(api, 'clGetKernelWorkGroupInfo', ['cl-api-get-kernel-work-group-info'])
add_plain_test(api, 'clRetainKernel and clReleaseKernel', ['cl-api-retain_release-kernel'])
add_plain_test(api, 'clSetKernelArg', ['cl-api-set-kernel-arg'])
#  Event
add_plain_test(api, 'clGetEventInfo', ['cl-api-get-event-info'])
add_plain_test(api, 'clRetainEvent and clReleaseEvent', ['cl-api-retain_release-event'])

# Program
add_plain_test(program, 'Run kernel with max work item sizes', ['cl-program-max-work-item-sizes'])
add_plain_test(program, 'Bitcoin: phatk kernel', ['cl-program-bitcoin-phatk'])

# Program tester

def add_program_test_dir(group, dirpath):
        for filename in os.listdir(dirpath):
                filepath = path.join(dirpath, filename)
                ext = filename.rsplit('.')[-1]
                if ext != 'cl' and ext != 'program_test':
                        continue
                testname = filename[0:-(len(ext) + 1)]
                add_plain_program_tester_test(group, testname, filepath)

program_build = {}
program_build_fail = {}
program_execute = {}
program["Build"] = program_build
program["Build"]["Fail"] = program_build_fail
program["Execute"] = program_execute

add_program_test_dir(program_build,
                     os.path.join(TESTS_DIR, 'cl', 'program', 'build'))
add_program_test_dir(program_build_fail,
                     os.path.join(TESTS_DIR, 'cl', 'program', 'build', 'fail'))
add_program_test_dir(program_execute,
                     os.path.join(TESTS_DIR, 'cl', 'program', 'execute'))
add_program_test_dir(program_execute,
                     os.path.join(TESTS_DIR, 'cl', 'program', 'execute',
                                  'builtin', 'atomic'))
add_program_test_dir(program_execute,
                     os.path.join(TESTS_DIR, 'cl', 'program', 'execute',
                                  'builtin', 'convert'))

#Run generated built-in tests
program_execute_builtin = {}
program["Execute"]["Builtin"] = program_execute_builtin
add_program_test_dir(program_execute_builtin,
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'builtin', 'int'))
add_program_test_dir(program_execute_builtin,
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'builtin', 'math'))
add_program_test_dir(program_execute_builtin,
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'builtin',
                                  'relational'))
program_execute_store = {}
program["Execute"]["Store"] = program_execute_store
add_program_test_dir(program_execute_store,
                     os.path.join(GENERATED_TESTS_DIR, 'cl', 'store'))
