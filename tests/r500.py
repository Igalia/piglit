#
# Testing the r500 DRI driver
#

from test.quick import profile

__all__ = ['profile']

# glean/blendFunc
#   R500 blending hardware appears to be a bit better than R300
env = profile.tests['glean']['blendFunc'].env
env['GLEAN_BLEND_RGB_TOLERANCE'] = '1.35'
env['GLEAN_BLEND_ALPHA_TOLERANCE'] = '1.35'
