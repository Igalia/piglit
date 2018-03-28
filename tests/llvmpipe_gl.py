# -*- coding: utf-8 -*-

from __future__ import (
    absolute_import, division, print_function, unicode_literals
)

import platform
import sys

from framework.grouptools import join
from tests.quick_gl import profile as _profile

__all__ = ['profile']

profile = _profile.copy()  # pylint: disable=invalid-name


def remove(key):
    try:
        del profile.test_list[key]
    except KeyError:
        sys.stderr.write('warning: test %s does not exist\n' % key)
        sys.stderr.flush()


# These take too long or too much memory
remove(join('spec', '!OpenGL 1.0', 'gl-1.0-blend-func'))
remove(join('spec', '!OpenGL 1.1', 'streaming-texture-leak'))
remove(join('spec', '!OpenGL 1.1', 'max-texture-size'))
remove(join('spec', 'ext_texture_env_combine', 'ext_texture_env_combine-combine'))

if platform.system() != 'Windows':
    remove(join('glx', 'glx-multithread-shader-compile'))
