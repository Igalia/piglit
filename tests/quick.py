# -*- coding: utf-8 -*-

from framework.gleantest import GleanTest
from tests.all import profile

__all__ = ['profile']

GleanTest.GLOBAL_PARAMS += ["--quick"]

# These take too long
del profile.tests['shaders']['glsl-fs-inline-explosion']
del profile.tests['shaders']['glsl-fs-unroll-explosion']
del profile.tests['shaders']['glsl-vs-inline-explosion']
del profile.tests['shaders']['glsl-vs-unroll-explosion']
