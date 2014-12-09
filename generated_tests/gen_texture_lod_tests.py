# Copyright (c) 2014 Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

""" Generate spec/ARB_shader_texture_lod tests """

from __future__ import print_function
import os
import collections

from templates import template_dir

TEMPLATES = template_dir(os.path.basename(os.path.splitext(__file__)[0]))

Parameters = collections.namedtuple(
    'Parameters', ['coord', 'grad', 'dimensions', 'mode'])


LOD_TESTS = [
    Parameters('float', 'float', '1D', 'texture1D'),
    Parameters('vec2', 'float', '1D', 'texture1DProj'),
    Parameters('vec4', 'float', '1D', 'texture1DProj'),
    Parameters('vec3', 'float', '1DShadow', 'shadow1D'),
    Parameters('vec4', 'float', '1DShadow', 'shadow1DProj'),
    Parameters('vec2', 'vec2', '2D', 'texture2D'),
    Parameters('vec3', 'vec2', '2D', 'texture2DProj'),
    Parameters('vec4', 'vec2', '2D', 'texture2DProj'),
    Parameters('vec3', 'vec2', '2DShadow', 'shadow2D'),
    Parameters('vec4', 'vec2', '2DShadow', 'shadow2DProj'),
    Parameters('vec3', 'vec3', '3D', 'texture3D'),
    Parameters('vec4', 'vec3', '3D', 'texture3DProj'),
    Parameters('vec3', 'vec3', 'Cube', 'textureCube')
]

GRAD_TESTS = LOD_TESTS + [
    Parameters('vec2', 'vec2', '2DRect', 'texture2DRect'),
    Parameters('vec3', 'vec2', '2DRect', 'texture2DRectProj'),
    Parameters('vec3', 'vec2', '2DRectShadow', 'shadow2DRect'),
    Parameters('vec4', 'vec2', '2DRectShadow', 'shadow2DRectProj')
]


def get_extensions(mode):
    """ If this test uses GL_ARB_texture_rectangle add it

    GL_ARB_texture_rectangle is an odd extension, it is on by default, so don't
    generate a #extension in the shader, just in the config block.

    """
    if 'Rect' in mode:
        return 'GL_ARB_shader_texture_lod GL_ARB_texture_rectangle'
    return 'GL_ARB_shader_texture_lod'


def main():
    """ Main function

    Writes tests to generated_tests/spec/arb_shader_texture_lod/ directory

    """
    dirname = 'spec/arb_shader_texture_lod/compiler'
    if not os.path.exists(dirname):
        os.makedirs(dirname)

    for params in LOD_TESTS:
        name = os.path.join(
            dirname,
            "tex_lod-{mode}-{dimensions}-{coord}.frag".format(
                mode=params.mode,
                dimensions=params.dimensions,
                coord=params.coord))
        print(name)
        with open(name, 'w+') as f:
            f.write(TEMPLATES.get_template(
                'frag_lod.glsl_parser_test.mako').render_unicode(param=params))

    for params in GRAD_TESTS:
        # Generate fragment shader test
        name = os.path.join(
            dirname,
            "tex_grad-{mode}-{dimensions}-{coord}".format(
                mode=params.mode,
                dimensions=params.dimensions,
                coord=params.coord))

        for stage in ['frag', 'vert']:
            print('{0}.{1}'.format(name, stage))
            with open('{0}.{1}'.format(name, stage), 'w+') as f:
                f.write(TEMPLATES.get_template(
                    'tex_grad.{0}.mako'.format(stage)).render_unicode(
                        param=params,
                        extensions=get_extensions(params.mode)))


if __name__ == '__main__':
    main()
