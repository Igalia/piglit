# coding=utf-8
#
# Copyright © 2013, 2014 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.

"""Generate interpolation-qualifier tests."""

from __future__ import print_function
import os
import itertools

from templates import template_dir
from modules import utils

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

INTERPOLATION_MODES = [
    'flat',
    'noperspective',
    'smooth',
    'default'
]

VS_VARIABLES = [
    'gl_FrontColor',
    'gl_BackColor',
    'gl_FrontSecondaryColor',
    'gl_BackSecondaryColor'
]

VS_VARIABLES_FRONT_ONLY = [
    'gl_FrontColor',
    'gl_FrontSecondaryColor',
]

OTHER_SIDE_MAP = {
    'gl_FrontColor': 'gl_BackColor',
    'gl_BackColor': 'gl_FrontColor',
    'gl_FrontSecondaryColor': 'gl_BackSecondaryColor',
    'gl_BackSecondaryColor': 'gl_FrontSecondaryColor'
}

VS_TO_FS_VARIABLE_MAP = {
    'gl_FrontColor': 'gl_Color',
    'gl_BackColor': 'gl_Color',
    'gl_FrontSecondaryColor': 'gl_SecondaryColor',
    'gl_BackSecondaryColor': 'gl_SecondaryColor'
}


def make_fs_vs_tests(fs_mode, vs_mode, dirname):
    if vs_mode == fs_mode:
        return

    for var in VS_VARIABLES:
        filename = os.path.join(
            dirname,
            '{0}-{1}-{2}-{3}.shader_test'.format(
                vs_mode, var, fs_mode,
                VS_TO_FS_VARIABLE_MAP[var]))
        print(filename)

        with open(filename, 'w') as f:
            f.write(TEMPLATES.get_template('vs-fs.shader_test.mako').render(
                vs_mode=vs_mode,
                vs_variable=var,
                fs_mode=fs_mode,
                fs_variable=VS_TO_FS_VARIABLE_MAP[var]))


def make_vs_unused_tests(vs_mode, dirname):
    if vs_mode == 'default':
        return

    for var in VS_VARIABLES:
        filename = os.path.join(
            dirname,
            '{0}-{1}-unused-{2}.shader_test'.format(
                vs_mode, var,
                VS_TO_FS_VARIABLE_MAP[var]))
        print(filename)

        with open(filename, 'w') as f:
            f.write(
                TEMPLATES.get_template('vs-unused.shader_test.mako').render(
                    vs_mode=vs_mode,
                    vs_variable=var))


def make_fs_unused_tests(fs_mode, vs_mode, dirname):
    if fs_mode == 'default':
        return

    for var in VS_VARIABLES_FRONT_ONLY:
        filename = os.path.join(
            dirname,
            'unused-{0}-{1}-{2}.shader_test'.format(
                var, fs_mode,
                VS_TO_FS_VARIABLE_MAP[var]))
        print(filename)

        with open(filename, 'w') as f:
            f.write(TEMPLATES.get_template('fs-unused.shader_test.mako').render(
                vs_mode=vs_mode,
                vs_variable=var,
                fs_mode=fs_mode,
                fs_variable=VS_TO_FS_VARIABLE_MAP[var]))


def make_vs_fs_unused_tests(fs_mode, vs_mode, dirname):
    if vs_mode == fs_mode:
        return

    for var in VS_VARIABLES:
        filename = os.path.join(
            dirname,
            'unused-{0}-{1}-unused-{2}-{3}.shader_test'.format(
                vs_mode, var, fs_mode,
                VS_TO_FS_VARIABLE_MAP[var]))
        print(filename)

        with open(filename, 'w') as f:
            f.write(TEMPLATES.get_template(
                'fs-vs-unused.shader_test.mako').render(
                    vs_mode=vs_mode,
                    vs_variable=var,
                    fs_mode=fs_mode,
                    fs_variable=VS_TO_FS_VARIABLE_MAP[var]))


def make_vs_fs_flip_tests(fs_mode, vs_mode, dirname):
    if vs_mode == fs_mode:
        return

    for this_side in VS_VARIABLES:
        other_side = OTHER_SIDE_MAP[this_side]
        filename = os.path.join(
            dirname,
            '{0}-{1}-{2}-{3}.shader_test'.format(
                vs_mode, this_side, fs_mode, other_side))
        print(filename)

        with open(filename, 'w') as f:
            f.write(TEMPLATES.get_template(
                'vs-fs-flip.shader_test.mako').render(
                    vs_mode=vs_mode,
                    this_side_variable=this_side,
                    other_side_variable=other_side,
                    fs_mode=fs_mode,
                    fs_variable=VS_TO_FS_VARIABLE_MAP[this_side]))


def main():
    """main function."""
    dirname = os.path.join('spec', 'glsl-1.30', 'linker',
                           'interpolation-qualifiers')
    utils.safe_makedirs(dirname)

    for fs_mode, vs_mode in itertools.product(INTERPOLATION_MODES, repeat=2):
        make_fs_vs_tests(fs_mode, vs_mode, dirname)
        make_vs_unused_tests(vs_mode, dirname)
        make_fs_unused_tests(fs_mode, vs_mode, dirname)
        make_vs_fs_unused_tests(fs_mode, vs_mode, dirname)
        make_vs_fs_flip_tests(fs_mode, vs_mode, dirname)


if __name__ == '__main__':
    main()
