# -*- coding: utf-8 -*-

import platform

from framework.grouptools import join
from tests.gpu import profile

__all__ = ['profile']


def remove(key):
    try:
        del profile.test_list[key]
    except KeyError:
        pass


# These take too long or too much memory
remove(join('glean', 'pointAtten'))
remove(join('glean', 'texCombine'))
profile.tests['spec']['!OpenGL 1.1'].pop('streaming-texture-leak', None)
profile.tests['spec']['!OpenGL 1.1'].pop('max-texture-size', None)

if platform.system() != 'Windows':
    profile.tests['glx'].pop('glx-multithread-shader-compile', None)
