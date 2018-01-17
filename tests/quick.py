# -*- coding: utf-8 -*-

"""A quicker profile than all.

This profile filters out a number of very slow tests, and tests that are very
exhaustively tested, since they add a good deal of runtime to piglit.

There are 18000+ auto-generated tests that are exhaustive, but for quick.py we
don't want that level of exhaustiveness, so this filter removes 80% in a random
(but deterministic) way.
"""

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)
import random

from framework import grouptools
from framework.test import PiglitGLTest
from tests.all import profile as _profile

__all__ = ['profile']

# See the note in all.py about this warning
# pylint: disable=bad-continuation


class FilterVsIn(object):
    """Filter out 80% of the Vertex Attrib 64 vs_in tests."""

    def __init__(self):
        self.random = random.Random()
        self.random.seed(42)

    def __call__(self, name, _):
        if 'vs_in' in name:
            # 20%
            return self.random.random() <= .2
        return True


profile = _profile.copy()  # pylint: disable=invalid-name

# Set the --quick flag on a few image_load_store_tests
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_image_load_store')) as g:
    with profile.test_list.allow_reassignment:
        g(['arb_shader_image_load_store-coherency', '--quick'], 'coherency')
        g(['arb_shader_image_load_store-host-mem-barrier', '--quick'],
          'host-mem-barrier')
        g(['arb_shader_image_load_store-max-size', '--quick'], 'max-size')
        g(['arb_shader_image_load_store-semantics', '--quick'], 'semantics')
        g(['arb_shader_image_load_store-shader-mem-barrier', '--quick'],
          'shader-mem-barrier')

# Set the --quick flag on the image_size test
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_image_size')) as g:
    with profile.test_list.allow_reassignment:
        g(['arb_shader_image_size-builtin', '--quick'], 'builtin')

# Set the --quick flag on the texture env combine test
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ext_texture_env_combine')) as g:
    with profile.test_list.allow_reassignment:
        g(['ext_texture_env_combine-combine', '--quick'], 'texture-env-combine')

# Set the --quick flag on the gl-1.0 blending test
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!opengl 1.0')) as g:
    with profile.test_list.allow_reassignment:
        g(['gl-1.0-blend-func', '--quick'], 'gl-1.0-blend-func')

# Limit texture size to 512x512 for some texture_multisample tests.
# The default (max supported size) can be pretty slow.
with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'ARB_texture_multisample')) as g:
    with profile.test_list.allow_reassignment:
        size_arg = ['--texsize', '512']
        g(['arb_texture_multisample-large-float-texture'] + size_arg,
          'large-float-texture', run_concurrent=False)
        g(['arb_texture_multisample-large-float-texture', '--array'] +
          size_arg, 'large-float-texture-array', run_concurrent=False)
        g(['arb_texture_multisample-large-float-texture', '--fp16'] +
          size_arg, 'large-float-texture-fp16', run_concurrent=False)
        g(['arb_texture_multisample-large-float-texture', '--array',
           '--fp16'] + size_arg,
          'large-float-texture-array-fp16', run_concurrent=False)

# These take too long
profile.filters.append(lambda n, _: '-explosion' not in n)
profile.filters.append(FilterVsIn())
