# coding=utf-8
#
# Copyright (C) 2016 Intel Corporation
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
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
# IN THE SOFTWARE.

"""Generate a set of tests to verify that formats added by the
 NV_image_formats extension for GLES 3.1 are properly accepted by the GLSL
 compiler.

 This also verifies that 16bits normalized formats are not accepted unless
 EXT_texture_norm16 is available.

 This program outputs, to stdout, the name of each file it generates.
"""

import os.path
from textwrap import dedent

from mako.template import Template

from modules import utils

def gen_header(status, norm16):
    """
    Generate a GLSL program header.

    Generate header code for ARB_shader_image_load_store GLSL parser
    tests that are expected to give status as result.
    """

    if norm16:
        norm16_require = "GL_EXT_texture_norm16"
    else:
        norm16_require = "!GL_EXT_texture_norm16"

    return dedent("""
        /*
         * [config]
         * expect_result: {0}
         * glsl_version: 3.10 es
         * require_extensions: GL_NV_image_formats {1}
         * [end config]
         */
        #version 310 es
        #extension GL_NV_image_formats : require
    """.format(status, norm16_require))


def gen(name, src, tests):
    """
    Expand a source template for the provided list of test definitions.

    Generate a GLSL parser test for each of the elements of the
    'tests' iterable, each of them should be a dictionary of
    definitions that will be used as environment to render the source
    template.

    The file name of each test will be the concatenation of the 'name'
    argument with the 'name' item from the respective test dictionary.
    """
    template = Template(dedent(src))

    for t in product([{'name': name}], tests):
        filename = os.path.join('spec',
                                'nv_image_formats',
                                'compiler',
                                '{0}.{1}'.format(t['name'],
                                                 t['shader_stage']))
        print(filename)

        dirname = os.path.dirname(filename)
        utils.safe_makedirs(dirname)

        with open(filename, 'w') as f:
            f.write(template.render(header=gen_header, **t))


shader_stages = [
    {'shader_stage': 'frag'},
    {'shader_stage': 'vert'}
]


image_types = [
    {
        'name': '2d',
        'image_type': 'image2D',
    },
    {
        'name': '3d',
        'image_type': 'image3D',
    },
    {
        'name': '2d-array',
        'image_type': 'image2DArray',
    },
    {
        'name': 'cube',
        'image_type': 'imageCube',
    }
]


def product(ps, *qss):
    """
    Generate the cartesian product of a number of lists of dictionaries.

    Each generated element will be the union of some combination of
    elements from the iterable arguments.  The resulting value of each
    'name' item will be the concatenation of names of the respective
    element combination separated with dashes.
    """
    for q in (product(*qss) if qss else [{}]):
        for p in ps:
            r = dict(p, **q)
            r['name'] = '-'.join(s['name'] for s in (p, q) if s.get('name'))
            yield r


def main():
    """Main function."""


    #
    # Test image formats accepted with NV_image_formats &
    # EXT_texture_norm16.
    #
    gen('declarations-with-norm16', """
        ${header('pass', True)}

        layout(rg32f) readonly uniform highp ${image_type} img_rg32f;
        layout(rg16f) readonly uniform highp ${image_type} img_rg16f;
        layout(r16f) readonly uniform highp ${image_type} img_r16f;
        layout(r11f_g11f_b10f) readonly uniform highp ${image_type} img_r11g11b10f;

        layout(rgba16) readonly uniform highp ${image_type} img_rgba16_unorm;
        layout(rgb10_a2) readonly uniform highp ${image_type} img_rgb10_a2_unorm;
        layout(rg16) readonly uniform highp ${image_type} img_rg16_unorm;
        layout(rg8) readonly uniform highp ${image_type} img_rg8_unorm;
        layout(r16) readonly uniform highp ${image_type} img_r16_unorm;
        layout(r8) readonly uniform highp ${image_type} img_r8_unorm;

        layout(rgba16_snorm) readonly uniform highp ${image_type} img_rgba16_snorm;
        layout(rg16_snorm) readonly uniform highp ${image_type} img_rg16_snorm;
        layout(rg8_snorm) readonly uniform highp ${image_type} img_rg8_snorm;
        layout(r16_snorm) readonly uniform highp ${image_type} img_r16_snorm;
        layout(r8_snorm) readonly uniform highp ${image_type} img_r8_snorm;

        layout(rgb10_a2ui) readonly uniform highp u${image_type} img_rgb10_a2ui;
        layout(rg32ui) readonly uniform highp u${image_type} img_rg32ui;
        layout(rg16ui) readonly uniform highp u${image_type} img_rg16ui;
        layout(rg8ui) readonly uniform highp u${image_type} img_rg8ui;
        layout(r16ui) readonly uniform highp u${image_type} img_r16ui;
        layout(r8ui) readonly uniform highp u${image_type} img_r8ui;

        layout(rg32i) readonly uniform highp i${image_type} img_rg32i;
        layout(rg16i) readonly uniform highp i${image_type} img_rg16i;
        layout(rg8i) readonly uniform highp i${image_type} img_rg8i;
        layout(r16i) readonly uniform highp i${image_type} img_r16i;
        layout(r8i) readonly uniform highp i${image_type} img_r8i;

        void main()
        {
        }
    """, product(image_types, shader_stages))


    #
    # Test image formats accepted with only NV_image_formats.
    #
    gen('declarations-without-norm16', """
        ${header('pass', False)}

        layout(rg32f) readonly uniform highp ${image_type} img_rg32f;
        layout(rg16f) readonly uniform highp ${image_type} img_rg16f;
        layout(r16f) readonly uniform highp ${image_type} img_r16f;
        layout(r11f_g11f_b10f) readonly uniform highp ${image_type} img_r11g11b10f;

        layout(rgb10_a2) readonly uniform highp ${image_type} img_rgb10_a2_unorm;
        layout(rg8) readonly uniform highp ${image_type} img_rg8_unorm;
        layout(r8) readonly uniform highp ${image_type} img_r8_unorm;

        layout(rg8_snorm) readonly uniform highp ${image_type} img_rg8_snorm;
        layout(r8_snorm) readonly uniform highp ${image_type} img_r8_snorm;

        layout(rgb10_a2ui) readonly uniform highp u${image_type} img_rgb10_a2ui;
        layout(rg32ui) readonly uniform highp u${image_type} img_rg32ui;
        layout(rg16ui) readonly uniform highp u${image_type} img_rg16ui;
        layout(rg8ui) readonly uniform highp u${image_type} img_rg8ui;
        layout(r16ui) readonly uniform highp u${image_type} img_r16ui;
        layout(r8ui) readonly uniform highp u${image_type} img_r8ui;

        layout(rg32i) readonly uniform highp i${image_type} img_rg32i;
        layout(rg16i) readonly uniform highp i${image_type} img_rg16i;
        layout(rg8i) readonly uniform highp i${image_type} img_rg8i;
        layout(r16i) readonly uniform highp i${image_type} img_r16i;
        layout(r8i) readonly uniform highp i${image_type} img_r8i;

        void main()
        {
        }
    """, product(image_types, shader_stages))


    #
    # Test image formats forbidden without EXT_texture_norm16.
    #
    gen('declaration-disallow-rgba16-unorm', """
        ${header('fail', False)}

        layout(rgba16) readonly uniform highp ${image_type} img_rgba16;

        void main()
        {
        }
    """, product(image_types, shader_stages))


    gen('declaration-disallow-rg16-unorm', """
        ${header('fail', False)}

        layout(rg16) readonly uniform highp ${image_type} img_rg16;

        void main()
        {
        }
    """, product(image_types, shader_stages))


    gen('declaration-disallow-r16-unorm', """
        ${header('fail', False)}

        layout(r16) readonly uniform highp ${image_type} img_r16;

        void main()
        {
        }
    """, product(image_types, shader_stages))


    gen('declaration-disallow-rgba16-snorm', """
        ${header('fail', False)}

        layout(rgba16_snorm) readonly uniform highp ${image_type} img_rgba16_snorm;

        void main()
        {
        }
    """, product(image_types, shader_stages))


    gen('declaration-disallow-rg16-snorm', """
        ${header('fail', False)}

        layout(rg16_snorm) readonly uniform highp ${image_type} img_rg16_snorm;

        void main()
        {
        }
    """, product(image_types, shader_stages))


    gen('declaration-disallow-r16-snorm', """
        ${header('fail', False)}

        layout(r16_snorm) readonly uniform highp ${image_type} img_r16_snorm;

        void main()
        {
        }
    """, product(image_types, shader_stages))

if __name__ == '__main__':
    main()
