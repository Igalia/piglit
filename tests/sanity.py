#
# Minimal tests to check whether the installation is working
#

from framework.core import Group, TestProfile
from framework.gleantest import GleanTest

__all__ = ['profile']

glean = Group()
glean['basic'] = GleanTest('basic')
glean['readPixSanity'] = GleanTest('readPixSanity')

profile = TestProfile()
profile.tests['glean'] = glean
