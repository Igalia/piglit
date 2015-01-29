# -*- coding: utf-8 -*-

from framework import grouptools
from framework.test import (GleanTest, PiglitGLTest)
from tests.all import profile

__all__ = ['profile']

GleanTest.GLOBAL_PARAMS += ["--quick"]

# Set the --quick flag on a few image_load_store_tests
with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', 'arb_shader_image_load_store')) as g:
    g(['arb_shader_image_load_store-coherency', '--quick'], 'coherency')
    g(['arb_shader_image_load_store-host-mem-barrier', '--quick'],
      'host-mem-barrier')
    g(['arb_shader_image_load_store-max-size', '--quick'], 'max-size')
    g(['arb_shader_image_load_store-semantics', '--quick'], 'semantics')
    g(['arb_shader_image_load_store-shader-mem-barrier', '--quick'],
      'shader-mem-barrier')

# These take too long
profile.filter_tests(lambda n, _: '-explosion' not in n)
