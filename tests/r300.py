#
# Testing the r300 DRI driver
#

from framework.profile import TestProfile
from framework.test import PiglitGLTest

__all__ = ['profile']

profile = TestProfile()

profile.tests['spec/!OpenGL 1.0/gl-1.0-blend-func'] = \
        PiglitGLTest('gl-1.0-blend-func', run_concurrent=True)
env = profile.tests['spec/!OpenGL 1.0/gl-1.0-blend-func'].env
#   R300 blending hardware appears to be bad
# Note that a setting of 1 bit is a special 
# case in Piglit that explicitly sets tolerance = 1.0f.
env['PIGLIT_BLEND_RGB_TOLERANCE'] = '2' # bits
env['PIGLIT_BLEND_ALPHA_TOLERANCE'] = '2' # bits
