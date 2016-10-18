#
# Minimal tests to check whether the installation is working
#

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

from framework import grouptools
from framework.profile import TestProfile
from framework.test import PiglitGLTest

__all__ = ['profile']

profile = TestProfile()

with profile.test_list.group_manager(
        PiglitGLTest,
        grouptools.join('spec', '!OpenGL 1.0')) as g:
    g(['gl-1.0-readpixsanity'], run_concurrent=True)
