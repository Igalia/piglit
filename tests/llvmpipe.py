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
remove(join('spec', '!OpenGL 1.1', 'streaming-texture-leak'))
remove(join('spec', '!OpenGL 1.1', 'max-texture-size'))

if platform.system() != 'Windows':
    remove(join('glx', 'glx-multithread-shader-compile'))
