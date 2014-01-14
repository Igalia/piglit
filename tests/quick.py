# -*- coding: utf-8 -*-

# XXX: These lines must be the first statements for the GleanTest.globalParams
# assignment to be effective.  Do NOT add any import statement before.
from framework.gleantest import GleanTest
GleanTest.globalParams += ["--quick"]

from tests.all import profile

__all__ = ['profile']

# These take too long
del profile.tests['shaders']['glsl-fs-inline-explosion']
del profile.tests['shaders']['glsl-fs-unroll-explosion']
del profile.tests['shaders']['glsl-vs-inline-explosion']
del profile.tests['shaders']['glsl-vs-unroll-explosion']
