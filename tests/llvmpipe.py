# -*- coding: utf-8 -*-

import os.path
import platform
from tests.gpu import profile

__all__ = ['profile']

# These take too long or too much memory
profile.tests['glean'].pop('pointAtten', None)
profile.tests['glean'].pop('texCombine', None)
profile.tests['spec']['!OpenGL 1.1'].pop('streaming-texture-leak', None)
profile.tests['spec']['!OpenGL 1.1'].pop('max-texture-size', None)

if platform.system() != 'Windows':
    profile.tests['glx'].pop('glx-multithread-shader-compile', None)
