#
# Minimal tests to check whether the installation is working
#

from framework import grouptools
from framework.profile import TestProfile
from framework.test import PiglitGLTest

__all__ = ['profile']

profile = TestProfile()

with profile.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!OpenGL 1.0', 'gl-1.0-readpixsanity')) as g:
    g(['gl-1.0-readpixsanity'], run_concurrent=True)
