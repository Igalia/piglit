# -*- coding: utf-8 -*-

from __future__ import (
     absolute_import, division, print_function, unicode_literals
)

import six

from tests.gpu import profile as _profile
from framework.test import PiglitGLTest
from framework.test.shader_test import ShaderTest, MultiShaderTest
from framework.profile import TestDict

__all__ = ['profile']

profile = _profile.copy()  # pylint: disable=invalid-name

# Save the old test_list, but create a new one to contain the modified tests
old_test_list = profile.test_list
profile.test_list = TestDict()

# Add a modified version of each PiglitGLTest as a khr_no_error variant.
# Shader runner doesn't explitly test for expected errors so we add shader
# tests as is. We actively filter GLSL parser and any other type of tests.
for name, test in six.iteritems(old_test_list):
    if isinstance(test, (PiglitGLTest, ShaderTest, MultiShaderTest)):
        profile.test_list['{} khr_no_error'.format(name)] = test
        test.command += ['-khr_no_error']
