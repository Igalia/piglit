#
# Minimal tests to check whether the installation is working
#

from framework.profile import TestProfile
from framework.test import GleanTest

__all__ = ['profile']

profile = TestProfile()
profile.tests['glean/basic'] = GleanTest('basic')
profile.tests['glean/readPixSanity'] = GleanTest('readPixSanity')
