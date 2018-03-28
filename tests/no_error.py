# -*- coding: utf-8 -*-

from __future__ import (
     absolute_import, division, print_function, unicode_literals
)
import itertools

import six

from tests.quick_gl import profile as _profile1
from tests.quick_shader import profile as _profile2
from framework.test import PiglitGLTest
from framework.test.shader_test import ShaderTest, MultiShaderTest
from framework.profile import TestProfile

__all__ = ['profile']

# Save the filters from the original profiles to a new profile
profile = TestProfile()
profile.filters = _profile1.filters + _profile2.filters

# Add a modified version of each PiglitGLTest as a khr_no_error variant.
# Shader runner doesn't explitly test for expected errors so we add shader
# tests as is. We actively filter GLSL parser and any other type of tests.
for name, test in itertools.chain(six.iteritems(_profile1.test_list),
                                  six.iteritems(_profile2.test_list)):
    if isinstance(test, (PiglitGLTest, ShaderTest, MultiShaderTest)):
        profile.test_list['{} khr_no_error'.format(name)] = test
        test.command += ['-khr_no_error']
