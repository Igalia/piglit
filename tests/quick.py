# -*- coding: utf-8 -*-

from framework.test import (GleanTest, PiglitGLTest)
from tests.all import profile

__all__ = ['profile']

GleanTest.GLOBAL_PARAMS += ["--quick"]

profile.tests['spec']['ARB_shader_image_load_store']['coherency'] = PiglitGLTest(
    ['arb_shader_image_load_store-coherency', '--quick'], run_concurrent=True)
profile.tests['spec']['ARB_shader_image_load_store']['host-mem-barrier'] = PiglitGLTest(
    ['arb_shader_image_load_store-host-mem-barrier', '--quick'], run_concurrent=True)
profile.tests['spec']['ARB_shader_image_load_store']['max-size'] = PiglitGLTest(
    ['arb_shader_image_load_store-max-size', '--quick'], run_concurrent=True)
profile.tests['spec']['ARB_shader_image_load_store']['semantics'] = PiglitGLTest(
    ['arb_shader_image_load_store-semantics', '--quick'], run_concurrent=True)
profile.tests['spec']['ARB_shader_image_load_store']['shader-mem-barrier'] = PiglitGLTest(
    ['arb_shader_image_load_store-shader-mem-barrier', '--quick'], run_concurrent=True)

# These take too long
profile.filter_tests(lambda n, _: '-explosion' not in n)
