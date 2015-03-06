# -*- coding: utf-8 -*-

import platform
import sys

from framework.grouptools import join
from tests.gpu import profile

__all__ = ['profile']


def remove(key):
    try:
        del profile.test_list[key]
    except KeyError:
        sys.stderr.write('warning: test %s does not exist\n' % key)
        sys.stderr.flush()


# These take too long or too much memory
remove(join('glean', 'pointAtten'))
remove(join('glean', 'texCombine'))
remove(join('spec', '!OpenGL 1.1', 'streaming-texture-leak'))
remove(join('spec', '!OpenGL 1.1', 'max-texture-size'))

if platform.system() != 'Windows':
    remove(join('glx', 'glx-multithread-shader-compile'))
