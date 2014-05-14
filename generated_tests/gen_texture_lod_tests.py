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
import textwrap
import collections
import mako.template

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
    """ If this test uses GL_ARB_texture_rectangle add it"""
    if 'Rect' in mode:
        return 'GL_ARB_shader_texture_lod GL_ARB_texture_rectangle'
    return 'GL_ARB_shader_texture_lod'


def gen_frag_lod_test(parameter, filename):
    """ Generate fragment shader LOD tests """
    template = mako.template.Template(textwrap.dedent("""
    /* [config]
     * expect_result: pass
     * glsl_version: 1.10
     * require_extensions: GL_ARB_shader_texture_lod
     * [end config]
     */
    #extension GL_ARB_shader_texture_lod: require

    uniform sampler${param.dimensions} s;
    varying ${param.coord} coord;
    varying float lod;

    void main()
    {
      gl_FragColor = ${param.mode}Lod(s, coord, lod);
    }
    """))
    with open(filename, 'w+') as f:
        f.write(template.render_unicode(param=parameter))


def gen_frag_grad_test(parameter, filename):
    """ Generate fragment shader gradient tests """
    template = mako.template.Template(textwrap.dedent("""
    /* [config]
     * expect_result: pass
     * glsl_version: 1.10
     * require_extensions: ${extensions}
     * [end config]
     */
    #extension GL_ARB_shader_texture_lod: require

    uniform sampler${param.dimensions} s;
    varying ${param.coord} coord;
    varying ${param.grad} dPdx;
    varying ${param.grad} dPdy;

    void main()
    {
      gl_FragColor = ${param.mode}GradARB(s, coord, dPdx, dPdy);
    }
    """))
    with open(filename, 'w+') as f:
        f.write(template.render_unicode(
            param=parameter,
            extensions=get_extensions(parameter.mode)))


def gen_vert_grad_test(parameter, filename):
    """ Generate vertex shader gradient tests """
    template = mako.template.Template(textwrap.dedent("""
    /* [config]
     * expect_result: pass
     * glsl_version: 1.10
     * require_extensions: ${extensions}
     * [end config]
     */
    #extension GL_ARB_shader_texture_lod: require

    uniform sampler${param.dimensions} s;
    attribute vec4 pos;
    attribute ${param.coord} coord;
    attribute ${param.grad} dPdx;
    attribute ${param.grad} dPdy;
    varying vec4 color;

    void main()
    {
      gl_Position = pos;
      color = ${param.mode}GradARB(s, coord, dPdx, dPdy);
    }
    """))
    with open(filename, 'w+') as f:
        f.write(template.render_unicode(
            param=parameter,
            extensions=get_extensions(parameter.mode)))


def main():
    """ Main function

    Writes tests to generated_tests/spec/arb_shader_texture_lod/ directory

    """
    try:
        os.makedirs('spec/ARB_shader_texture_lod/compiler')
    except OSError:
        pass

    for count, params in enumerate(LOD_TESTS, start=1):
        name = ("spec/ARB_shader_texture_lod/compiler/"
                "tex_lod-{0:02d}.frag".format(count))
        print(name)
        gen_frag_lod_test(params, name)

    for count, params in enumerate(GRAD_TESTS, start=1):
        name = ("spec/ARB_shader_texture_lod/compiler/"
                "tex_grad-{0:02d}.frag".format(count))
        print(name)
        gen_frag_grad_test(params, name)

    # Start the count at len(GRAD_TESTS) + 1 so that the frag and vertex tests
    # are sequentially numbered.
    for count, params in enumerate(GRAD_TESTS, start=len(GRAD_TESTS) + 1):
        name = ("spec/ARB_shader_texture_lod/compiler/"
                "tex_grad-{0:02d}.vert".format(count))
        print(name)
        gen_vert_grad_test(params, name)


if __name__ == '__main__':
    main()
