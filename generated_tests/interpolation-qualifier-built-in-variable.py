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

from __future__ import print_function
import os

from templates import template_dir

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

interpolation_modes = [
    'flat',
    'noperspective',
    'smooth',
    'default'
]

vertex_shader_variables = [
    'gl_FrontColor',
    'gl_BackColor',
    'gl_FrontSecondaryColor',
    'gl_BackSecondaryColor'
]

vertex_shader_variables_front_only = [
    'gl_FrontColor',
    'gl_FrontSecondaryColor',
]

other_side_map = {
    'gl_FrontColor': 'gl_BackColor',
    'gl_BackColor': 'gl_FrontColor',
    'gl_FrontSecondaryColor': 'gl_BackSecondaryColor',
    'gl_BackSecondaryColor': 'gl_FrontSecondaryColor'
}

vertex_shader_to_fragment_shader_variable_map = {
    'gl_FrontColor': 'gl_Color',
    'gl_BackColor': 'gl_Color',
    'gl_FrontSecondaryColor': 'gl_SecondaryColor',
    'gl_BackSecondaryColor': 'gl_SecondaryColor'
}


for fs_mode in interpolation_modes:
    for vs_mode in interpolation_modes:
        if vs_mode == fs_mode:
            continue

        for var in vertex_shader_variables:
            filename = os.path.join(
                'spec',
                'glsl-1.30',
                'linker',
                'interpolation-qualifiers',
                '{0}-{1}-{2}-{3}.shader_test'.format(
                    vs_mode, var, fs_mode,
                    vertex_shader_to_fragment_shader_variable_map[var]))
            print(filename)

            dirname = os.path.dirname(filename)
            if not os.path.exists(dirname):
                os.makedirs(dirname)

            with open(filename, 'w') as f:
                f.write(TEMPLATES.get_template('vs-fs.shader_test.mako').render(
                    vs_mode=vs_mode,
                    vs_variable=var,
                    fs_mode=fs_mode,
                    fs_variable=vertex_shader_to_fragment_shader_variable_map[var]))


for vs_mode in interpolation_modes:
    if vs_mode == 'default':
        continue

    for var in vertex_shader_variables:
        filename = os.path.join(
            'spec',
            'glsl-1.30',
            'linker',
            'interpolation-qualifiers',
            '{0}-{1}-unused-{2}.shader_test'.format(vs_mode, var,
                vertex_shader_to_fragment_shader_variable_map[var]))
        print(filename)

        dirname = os.path.dirname(filename)
        if not os.path.exists(dirname):
            os.makedirs(dirname)

        with open(filename, 'w') as f:
            f.write(
                TEMPLATES.get_template('vs-unused.shader_test.mako').render(
                    vs_mode=vs_mode,
                    vs_variable=var))


for fs_mode in interpolation_modes:
    if fs_mode == 'default':
        continue

    for var in vertex_shader_variables_front_only:
        filename = os.path.join(
            'spec',
            'glsl-1.30',
            'linker',
            'interpolation-qualifiers',
            'unused-{0}-{1}-{2}.shader_test'.format(
                var, fs_mode,
                vertex_shader_to_fragment_shader_variable_map[var]))
        print(filename)

        dirname = os.path.dirname(filename)
        if not os.path.exists(dirname):
            os.makedirs(dirname)

        with open(filename, 'w') as f:
            f.write(TEMPLATES.get_template('fs-unused.shader_test.mako').render(
                vs_mode=vs_mode,
                vs_variable=var,
                fs_mode=fs_mode,
                fs_variable=vertex_shader_to_fragment_shader_variable_map[var]))



for fs_mode in interpolation_modes:
    for vs_mode in interpolation_modes:
        if vs_mode == fs_mode:
            continue

        for var in vertex_shader_variables:
            filename = os.path.join(
                'spec',
                'glsl-1.30',
                'linker',
                'interpolation-qualifiers',
                'unused-{0}-{1}-unused-{2}-{3}.shader_test'.format(
                    vs_mode, var, fs_mode,
                    vertex_shader_to_fragment_shader_variable_map[var]))
            print(filename)

            dirname = os.path.dirname(filename)
            if not os.path.exists(dirname):
                os.makedirs(dirname)

            with open(filename, 'w') as f:
                f.write(TEMPLATES.get_template(
                    'fs-vs-unused.shader_test.mako').render(
                        vs_mode=vs_mode,
                        vs_variable=var,
                        fs_mode=fs_mode,
                        fs_variable=vertex_shader_to_fragment_shader_variable_map[var]))



for fs_mode in interpolation_modes:
    for vs_mode in interpolation_modes:
        if vs_mode == fs_mode:
            continue

        for this_side in vertex_shader_variables:
            other_side = other_side_map[this_side]
            filename = os.path.join(
                'spec',
                'glsl-1.30',
                'linker',
                'interpolation-qualifiers',
                '{0}-{1}-{2}-{3}.shader_test'.format(
                    vs_mode, this_side, fs_mode, other_side))
            print(filename)

            dirname = os.path.dirname(filename)
            if not os.path.exists(dirname):
                os.makedirs(dirname)

            with open(filename, 'w') as f:
                f.write(TEMPLATES.get_template(
                    'vs-fs-flip.shader_test.mako').render(
                        vs_mode=vs_mode,
                        this_side_variable=this_side,
                        other_side_variable=other_side,
                        fs_mode=fs_mode,
                        fs_variable=vertex_shader_to_fragment_shader_variable_map[this_side]))
