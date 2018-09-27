# encoding=utf-8
# Copyright © 2018 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

"""A minimal profile that aims to touch test most major OpenGL features with as
few tests as possible.

This is largely aimed at bring up of a new driver or a driver on new hardware,
where performance and stability are not necessarily the best. The idea is to
touch the main functionality of important features without running for too
long. This *will* miss many corner cases, that's okay. For more stable
hardware or software stacks the "gpu" or "quick" profiles are probably more
appropriate.
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import os

from framework import grouptools
from framework.profile import TestProfile
from framework.test import PiglitGLTest
from framework.test.shader_test import ShaderTest
from .py_modules.constants import GENERATED_TESTS_DIR, TESTS_DIR

__all__ = ['profile']

basepath = os.path.normpath(os.path.join(TESTS_DIR, '..'))
gen_basepath = os.path.relpath(os.path.join(GENERATED_TESTS_DIR, '..'), basepath)


def add_shader_test(shader):
    """Given an adder, creates a group and adds a shader test."""
    abs_path = os.path.abspath(shader)
    if os.path.commonprefix([abs_path, GENERATED_TESTS_DIR]) == GENERATED_TESTS_DIR:
        installpath = os.path.relpath(shader, gen_basepath)
    else:
        installpath = None

    dirpath = os.path.dirname(shader)
    groupname = grouptools.from_path(os.path.relpath(
        dirpath, GENERATED_TESTS_DIR if installpath else TESTS_DIR))
    testname = os.path.splitext(os.path.basename(shader))[0]

    profile.test_list[grouptools.join(groupname, testname)] = \
        ShaderTest.new(shader, installpath)


profile = TestProfile()

shader_tests = (
    'tests/fast_color_clear/all-colors.shader_test',
    'tests/shaders/arb_texture_gather-miplevels.shader_test',
    'tests/shaders/glsl-algebraic-add-zero.shader_test',
    'tests/shaders/glsl-derivs-varyings.shader_test',
    'tests/shaders/glsl-fs-max-3.shader_test',
    'tests/shaders/glsl-fs-texture2d-branching.shader_test',
    'tests/shaders/glsl-vs-mat-mul-1.shader_test',
    'tests/spec/arb_arrays_of_arrays/execution/image_store/basic-imageStore-mixed-const-non-const-uniform-index.shader_test',
    'tests/spec/arb_arrays_of_arrays/execution/sampler/vs-struct-nonconst.shader_test',
    'tests/spec/arb_compute_shader/execution/basic-ssbo.shader_test',
    'tests/spec/arb_compute_shader/execution/shared-atomics.shader_test',
    'tests/spec/arb_derivative_control/execution/dfdx-dfdy.shader_test',
    'tests/spec/arb_gpu_shader5/execution/built-in-functions/fs-bitfieldReverse.shader_test',
    'tests/spec/arb_shader_ballot/execution/fs-readFirstInvocation-uint-loop.shader_test',
    'tests/spec/arb_shader_group_vote/cs-all.shader_test',
    'tests/spec/arb_shader_image_load_store/execution/write-to-rendered-image.shader_test',
    'tests/spec/arb_shader_subroutine/execution/two-subroutines-nested.shader_test',
    'tests/spec/arb_shader_texture_lod/execution/glsl-fs-shadow2DGradARB-cumulative.shader_test',
    'tests/spec/arb_tessellation_shader/execution/barrier-patch.shader_test',
    'tests/spec/arb_tessellation_shader/execution/sanity.shader_test',
    'tests/spec/glsl-1.50/execution/geometry-basic.shader_test',
    'tests/spec/oes_viewport_array/viewport-gs-write-simple.shader_test',
)

for shader_test in shader_tests:
    add_shader_test(shader_test)

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'glsl-1.50')) as g:
    g(['glsl-1.50-transform-feedback-builtins'], 'transform-feedback-builtins')
