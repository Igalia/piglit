# coding=utf-8
#
# Copyright © 2014-2016 Intel Corporation
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

from __future__ import print_function, division, absolute_import

import os.path
from textwrap import dedent

from mako.template import Template

from modules import utils


def product(ps, *qss):
    """
    Generate the cartesian product of a number of lists of dictionaries.

    Generate a sequence with every possible combination of elements of
    the specified lists of dictionaries.  Each element of the result
    will be the union of some combination of dictionaries from the
    lists given as argument.  The 'name' attribute of each dictionary
    given as input is concatenated with the names of other
    dictionaries part of the same combination to give the 'name'
    attribute of the result.
    """
    for q in (product(*qss) if qss else [{}]):
        for p in ps:
            r = dict(p, **q)
            r['name'] = ''.join(s['name'] for s in (p, q) if s.get('name'))
            yield r


def gen(src, tests):
    """
    Expand a source template for the provided list of test definitions.

    Generate a test script for each element of the 'tests' iterable,
    each of them should be a dictionary of definitions that will be
    used as environment to render the source template.

    The 'path' attribute of each dictionary gives the filename each
    test will be written to.
    """
    template = Template(dedent(src))

    for t in tests:
        print(t['path'])
        utils.safe_makedirs(os.path.dirname(t['path']))
        with open(t['path'], 'w') as f:
            f.write(template.render(**t))


def gen_execution(src, tests):
    """
    Generate a shader runner test for each element of the 'tests'
    iterable.
    """
    return gen(src,
               (dict(t, path = os.path.join(
                   'spec', t['extension'], 'execution',
                   t['name'] + '.shader_test')) for t in tests))


def gen_compiler(src, tests):
    """
    Generate a GLSL parser test for each element of the 'tests'
    iterable.
    """
    return gen(src,
               (dict(t, path = os.path.join(
                   'spec', t['extension'], 'compiler',
                   t['name'] + '.' + t['shader_stage'])) for t in tests))


#
# Common test definitions independent of the framebuffer fetch
# extension.
#
common_defs = [{# Allocate and bind a framebuffer object of the given
                # format and number of samples.
                'bind_fb': lambda fmt, samples = 0:
                   'fb ms {0} 250 250 {1}'.format(fmt, samples) if samples > 0 else
                   ('texture storage 0 2D {0} (1 250 250)\n'
                    'fb tex 2d 0').format(fmt) if fmt[-1] == 'I' else
                   ('texture rgbw 0 (250, 250) {0}\n'
                    'fb tex 2d 0').format(fmt),

                # Resolve the contents of a framebuffer previously
                # allocated with 'bind_fb' of the given format and
                # samples into a newly allocated single-sample
                # framebuffer.
                'resolve_fb': lambda fmt, samples:
                   '' if samples == 0 else
                   ('texture storage 0 2D {0} (1 250 250)\n'
                    'fb draw tex 2d 0\n'
                    'blit color nearest\n'
                    'fb read tex 2d 0\n').format(fmt) if fmt[-1] == 'I' else
                   ('texture rgbw 0 (250, 250) {0}\n'
                    'fb draw tex 2d 0\n'
                    'blit color linear\n'
                    'fb read tex 2d 0').format(fmt),

                # Blit the contents of the current read framebuffer
                # into the window system framebuffer. XXX - Requires
                # GL(ES) 3.0+ since glBlitFramebuffer doesn't exist on
                # earlier API versions.
                'display_fb': lambda api_version:
                   '' if api_version < 3.0 else
                   ('fb draw winsys\n'
                    'blit color linear'),

                # Declare a fragment color array set up for
                # framebuffer fetch with the specified number of
                # elements (n=0 indicates a single scalar output).
                #
                # The fragment color array can be accessed using the
                # 'frag_data' and 'last_frag_data' macros defined
                # below to make sure the generated test is valid
                # irrespective of the GLSL version.
                'decl_frag_data': lambda api_version, layout, n = 0, \
                                         t = 'vec4', p = 'mediump':
                   '' if api_version < 3.0 and not layout and t == 'vec4' \
                                           and p == 'mediump' else
                   layout + ' ' + p + ' ' + t + ' gl_LastFragData[gl_MaxDrawBuffers];' \
                                                       if api_version < 3.0 else
                   layout + ' inout ' + p + ' ' + t + ' fcolor;' if n == 0 else
                   layout + ' inout ' + p + ' ' + t + ' fcolor[{0}];'.format(n),

                'frag_data': lambda api_version, i = -1:
                   'gl_FragData[{0}]'.format(max(0, i)) if api_version < 3.0 else
                   'fcolor[{0}]'.format(i) if i >= 0 else
                   'fcolor',

                'last_frag_data': lambda api_version, i = -1:
                   'gl_LastFragData[{0}]'.format(max(0, i)) if api_version < 3.0 else
                   'fcolor[{0}]'.format(i) if i >= 0 else
                   'fcolor'}]


#
# Common test definitions for all supported extensions.
#
all_defs = list(product(common_defs,
                       [{'extension': 'EXT_shader_framebuffer_fetch',
                         'layout': '',
                         'barrier': ''},
                        {'extension': 'EXT_shader_framebuffer_fetch_non_coherent',
                         'layout': 'layout(noncoherent)',
                         'barrier': 'fbfetch barrier'}]))


def main():
    """Main function."""

    #
    # Test that the GL(ES) 2 gl_LastFragData built-in is not defined
    # in more recent GLSL versions.
    #
    gen_compiler("""\
        /*
         * [config]
         * expect_result: fail
         * glsl_version: 3.0 es
         * require_extensions: GL_${extension}
         * [end config]
         */
        #version 300 es
        #extension GL_${extension} : enable

        out highp vec4 color;

        void main()
        {
            color = gl_LastFragData[0];
        }
    """, product(all_defs,
                 [{'name': 'negative-gl_LastFragData-gles3',
                   'shader_stage': 'frag'}]))

    #
    # Test that the GL(ES) 2 gl_LastFragData built-in is read-only.
    # From the EXT_shader_framebuffer_fetch extension: "[...] fragment
    # shaders should use the read-only input array gl_LastFragData."
    #
    gen_compiler("""\
        /*
         * [config]
         * expect_result: fail
         * glsl_version: 1.0 es
         * require_extensions: GL_${extension}
         * [end config]
         */
        #version 100
        #extension GL_${extension} : enable

        void main()
        {
            gl_LastFragData[0] = vec4(1.0);
        }
    """, product(all_defs,
                 [{'name': 'negative-gl_LastFragData-write-gles2',
                   'shader_stage': 'frag'}]))

    #
    # Test that GL(ES) 3+ user-defined inout arrays are not accepted
    # in earlier GLSL versions.
    #
    gen_compiler("""\
         /*
          * [config]
          * expect_result: fail
          * glsl_version: 1.0 es
          * require_extensions: GL_${extension}
          * [end config]
          */
        #version 100
        #extension GL_${extension} : enable

        inout highp vec4 color;

        void main()
        {
            color += vec4(0.5);
        }
    """, product(all_defs,
                 [{'name': 'negative-inout-fragment-output-gles2',
                   'shader_stage': 'frag'}]))

    #
    # Test that redeclaring an existing built-in fragment output as
    # 'inout' leads to an error.
    #
    gen_compiler("""\
         /*
          * [config]
          * expect_result: fail
          * glsl_version: 3.0 es
          * require_extensions: GL_${extension}
          * [end config]
          */
        #version 300 es
        #extension GL_${extension} : enable

        inout highp float gl_FragDepth;

        void main()
        {
            gl_FragDepth += 0.5;
        }
    """, product(all_defs,
                 [{'name': 'negative-inout-gl_FragDepth-gles3',
                   'shader_stage': 'frag'}]))

    #
    # Test that vertex shader interface variables cannot be declared
    # 'inout'.
    #
    gen_compiler("""\
         /*
          * [config]
          * expect_result: fail
          * glsl_version: 3.0 es
          * require_extensions: GL_${extension}
          * [end config]
          */
        #version 300 es
        #extension GL_${extension} : enable

        inout highp vec4 vcolor;

        void main()
        {
        }
    """, product(all_defs,
                 [{'name': 'negative-inout-vertex-output-gles3',
                   'shader_stage': 'vert'}]))

    #
    # Test basic framebuffer fetch functionality.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= ${3.0 if api_version >= 3.0 else 1.0}
        %if samples > 0:
        INT GL_MAX_SAMPLES >= ${samples}
        %endif
        GL_${extension}

        [vertex shader passthrough]

        [fragment shader]
        #version ${'300 es' if api_version >= 3.0 else '100'}
        #extension GL_${extension} : enable

        ${decl_frag_data(api_version, layout)}

        void main()
        {
           ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                       vec4(0.5, 0.0, 0.0, 0.0);
        }

        [test]
        ${bind_fb('GL_RGBA8', samples)}

        clear color 0.0 0.0 1.0 0.0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        ${resolve_fb('GL_RGBA8', samples)}

        relative probe rect rgb (0, 0.0, 1.0, 1.0) (1.0, 0.0, 1.0)

        ${display_fb(api_version)}
    """, product(all_defs,
                 [{'name': 'simple-'}],
                 [{'name': 'ss-gles2', 'api_version': 2.0, 'samples': 0},
                  {'name': 'ss-gles3', 'api_version': 3.0, 'samples': 0},
                  {'name': 'ms2-gles3', 'api_version': 3.0, 'samples': 2},
                  {'name': 'ms8-gles3', 'api_version': 3.0, 'samples': 8},
                  {'name': 'ms16-gles3', 'api_version': 3.0, 'samples': 16}]))

    #
    # Test read-back from a framebuffer with non-uniform contents
    # rendered during a previous draw call.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= ${3.0 if api_version >= 3.0 else 1.0}
        %if samples > 0:
        INT GL_MAX_SAMPLES >= ${samples}
        %endif
        GL_${extension}

        [vertex shader]
        #version ${'300 es' if api_version >= 3.0 else '100'}

        ${'in' if api_version >= 3.0 else 'attribute'} highp vec4 vertex;
        ${'out' if api_version >= 3.0 else 'varying'} highp vec4 vcolor;

        void main()
        {
            gl_Position = vertex;
            // Transform the vertex coordinates so that the R and G
            // components of the vertex color range between 0.0 and 1.0.
            vcolor = vec4((1.0 + vertex.x) / 2.0,
                          (1.0 + vertex.y) / 2.0, 0.0, 1.0);
        }

        [fragment shader]
        #version ${'300 es' if api_version >= 3.0 else '100'}
        #extension GL_${extension} : enable

        ${'in' if api_version >= 3.0 else 'varying'} highp vec4 vcolor;
        ${decl_frag_data(api_version, layout, p=precision)}

        void main()
        {
            // The condition makes sure that the else branch is
            // taken for the top and right quadrants during the second
            // overdraw.
            if (${last_frag_data(api_version)}.x <= 0.5 &&
                ${last_frag_data(api_version)}.y <= 0.5) {
                ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                            vcolor;
            } else {
                // Will give a solid color as result different for
                // each quadrant, assuming that the first branch was
                // taken during the first pass.
                ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                            vec4((vcolor.x >= 0.5 ? 1.0 : 0.0) - vcolor.x,
                                                 (vcolor.y >= 0.5 ? 1.0 : 0.0) - vcolor.y,
                                                 0, 0);
            }
        }

        [test]
        ${bind_fb('GL_RGBA8', samples)}

        clear color 0.0 0.0 1.0 0.0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        ${resolve_fb('GL_RGBA8', samples)}

        relative probe rect rgb (0.55, 0.0, 0.45, 0.45) (1.0, 0.0, 1.0)
        relative probe rect rgb (0.0, 0.55, 0.45, 0.45) (0.0, 1.0, 1.0)
        relative probe rect rgb (0.55, 0.55, 0.45, 0.45) (1.0, 1.0, 1.0)

        ${display_fb(api_version)}
    """, product(all_defs,
                 [{'name': 'nonuniform-'}],
                 [{'name': 'ss-gles2', 'api_version': 2.0,
                   'samples': 0, 'precision': 'mediump'},
                  {'name': 'ss-gles2-redecl-highp', 'api_version': 2.0,
                   'samples': 0, 'precision': 'highp'},
                  {'name': 'ss-gles2-redecl-lowp', 'api_version': 2.0,
                   'samples': 0, 'precision': 'lowp'},
                  {'name': 'ss-gles3', 'api_version': 3.0,
                   'samples': 0, 'precision': 'mediump'},
                  {'name': 'ms2-gles3', 'api_version': 3.0,
                   'samples': 2, 'precision': 'mediump'},
                  {'name': 'ms8-gles3', 'api_version': 3.0,
                   'samples': 8, 'precision': 'mediump'},
                  {'name': 'ms16-gles3', 'api_version': 3.0,
                   'samples': 16, 'precision': 'mediump'}]))

    #
    # Test basic framebuffer fetch functionality in combination with
    # texturing.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= 3.00
        GL_${extension}

        [vertex shader passthrough]

        [fragment shader]
        #version 300 es
        #extension GL_${extension} : enable

        uniform sampler2D s;
        ${decl_frag_data(api_version, layout)}

        void main()
        {
            ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                        texelFetch(s, ivec2(gl_FragCoord), 0) / 4.0;
        }

        [test]
        ${bind_fb('GL_RGBA8')}

        texture rgbw 1 (250, 250) GL_RGBA8
        uniform int s 1

        clear color 0.5 0.0 0.0 0.0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        relative probe rect rgb (0.0, 0.0, 0.45, 0.45) (1.0, 0.0, 0.0)
        relative probe rect rgb (0.55, 0.0, 0.45, 0.45) (0.5, 0.5, 0.0)
        relative probe rect rgb (0.0, 0.55, 0.45, 0.45) (0.5, 0.0, 0.5)
        relative probe rect rgb (0.55, 0.55, 0.45, 0.45) (1.0, 0.5, 0.5)

        ${display_fb(api_version)}
    """, product(all_defs,
                 [{'name': 'texture-gles3',
                   'api_version': 3.0}]))

    #
    # Test non-uniform fragment discard dependent on the result read
    # back from the framebuffer.  The EXT_shader_framebuffer_fetch
    # multisample test will be skipped if the implementation doesn't
    # claim to support per-sample discard.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= 3.00
        GL_${extension}
        %if samples > 0:
        INT GL_MAX_SAMPLES >= ${samples}
        %endif
        INT GL_FRAGMENT_SHADER_DISCARDS_SAMPLES_EXT >= ${1 if samples else 0}

        [vertex shader]
        #version 300 es

        in highp vec4 vertex;
        out highp vec4 vcolor;

        void main()
        {
            gl_Position = vertex;
            // Transform the vertex coordinates so that the R and G
            // components of the vertex color range between 0.0 and 1.0.
            vcolor = vec4((1.0 + vertex.x) / 2.0,
                          (1.0 + vertex.y) / 2.0, 0.0, 1.0);
        }

        [fragment shader]
        #version 300 es
        #extension GL_${extension} : enable

        in highp vec4 vcolor;
        ${decl_frag_data(api_version, layout)}

        void main()
        {
            // The condition makes sure that the discard branch is
            // taken for the top and right quadrants during the second
            // overdraw.
            if (${last_frag_data(api_version)}.x <= 0.45 &&
                ${last_frag_data(api_version)}.y < 0.45)
                ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                            vec4(vcolor.x >= 0.5 ? 0.5 : 0.1,
                                                 vcolor.y >= 0.5 ? 0.5 : 0.1,
                                                 0.0, 0.0);
            else
                discard;
        }

        [test]
        ${bind_fb('GL_RGBA8', samples)}

        clear color 0.0 0.0 1.0 0.0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        ${resolve_fb('GL_RGBA8', samples)}

        relative probe rect rgb (0.0, 0.0, 0.45, 0.45) (0.2, 0.2, 1.0)
        relative probe rect rgb (0.55, 0.0, 0.45, 0.45) (0.5, 0.1, 1.0)
        relative probe rect rgb (0.0, 0.55, 0.45, 0.45) (0.1, 0.5, 1.0)
        relative probe rect rgb (0.55, 0.55, 0.45, 0.45) (0.5, 0.5, 1.0)

        ${display_fb(api_version)}
    """, product(all_defs,
                 [{'name': 'discard-gles3-',
                   'api_version': 3.0}],
                 [{'name': 'ss', 'samples': 0},
                  {'name': 'ms8', 'samples': 8}]))

    #
    # Test read-back from an integer framebuffer with non-uniform
    # contents rendered during a previous draw call.  Similar to the
    # "nonuniform" test above but using an integer framebuffer.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= 3.00
        GL_${extension}
        %if samples > 0:
        INT GL_MAX_SAMPLES >= ${samples}
        %endif

        [vertex shader]
        #version 300 es

        in highp vec4 vertex;
        out highp vec4 vcolor;

        void main()
        {
            gl_Position = vertex;
            // Transform the vertex coordinates so that the R and G
            // components of the vertex color range between 0.0 and 1.0.
            vcolor = vec4((1.0 + vertex.x) / 2.0,
                          (1.0 + vertex.y) / 2.0, 0.0, 1.0);
        }

        [fragment shader]
        #version 300 es
        #extension GL_${extension} : enable
        #define SCALE 100

        in highp vec4 vcolor;
        ${decl_frag_data(api_version, layout, t='ivec4')}

        void main()
        {
           if (${last_frag_data(api_version)}.x <= SCALE / 2 &&
               ${last_frag_data(api_version)}.y <= SCALE / 2)
              ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                          ivec4(vcolor * float(SCALE));
           else
              ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                          ivec4((vcolor.x >= 0.5 ? SCALE : 0)
                                                 - int(vcolor.x * float(SCALE)),
                                                (vcolor.y >= 0.5 ? SCALE : 0)
                                                 - int(vcolor.y * float(SCALE)),
                                                0, 0);
        }

        [test]
        ${bind_fb('GL_RGBA8I', samples)}

        clear color 0 0 1 0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        ${resolve_fb('GL_RGBA8I', samples)}

        relative probe rect rgba int (0.55, 0.0, 0.45, 0.45) (100, 0, 127, 100)
        relative probe rect rgba int (0.0, 0.55, 0.45, 0.45) (0, 100, 127, 100)
        relative probe rect rgba int (0.55, 0.55, 0.45, 0.45) (100, 100, 127, 100)
    """, product(all_defs,
                 [{'name': 'integer-gles3-'}],
                 [{'name': 'ss', 'samples': 0, 'api_version': 3.0},
                  {'name': 'ms2', 'samples': 2, 'api_version': 3.1},
                  {'name': 'ms8', 'samples': 8, 'api_version': 3.1}]))

    #
    # Test framebuffer fetch functionality in combination with
    # multiple render targets.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= ${3.0 if api_version >= 3.0 else 1.0}
        GL_${extension}

        [vertex shader passthrough]

        [fragment shader]
        #version ${'300 es' if api_version >= 3.0 else '100'}
        #extension GL_${extension} : enable

        ${decl_frag_data(api_version, layout, 4)}

        void main()
        {
           ${frag_data(api_version, 0)} = ${last_frag_data(api_version, 0)} +
                                          vec4(0.5, 0.0, 0.0, 0.0);
           ${frag_data(api_version, 1)} = ${last_frag_data(api_version, 1)} +
                                          vec4(0.0, 0.5, 0.0, 0.0);
           ${frag_data(api_version, 2)} = ${last_frag_data(api_version, 2)} +
                                          vec4(0.5, 0.5, 0.0, 0.0);
           ${frag_data(api_version, 3)} = ${last_frag_data(api_version, 3)} +
                                          vec4(0.0, 0.0, 0.5, 0.0);
        }

        [test]
        texture rgbw 0 (250, 250) GL_RGBA8
        texture rgbw 1 (250, 250) GL_RGBA8
        texture rgbw 2 (250, 250) GL_RGBA8
        texture rgbw 3 (250, 250) GL_RGBA8
        fb tex 2d 0 1 2 3

        clear color 0.0 0.0 0.5 0.0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        fb tex 2d 0
        relative probe rect rgb (0.0, 0.0, 1.0, 1.0) (1.0, 0.0, 0.5)

        fb tex 2d 1
        relative probe rect rgb (0.0, 0.0, 1.0, 1.0) (0.0, 1.0, 0.5)

        fb tex 2d 2
        relative probe rect rgb (0.0, 0.0, 1.0, 1.0) (1.0, 1.0, 0.5)

        fb tex 2d 3
        relative probe rect rgb (0.0, 0.0, 1.0, 1.0) (0.0, 0.0, 1.0)

        ${display_fb(api_version)}
    """, product(all_defs,
                 [{'name': 'mrt-'}],
                 [{'name': 'gles2', 'api_version': 2.0},
                  {'name': 'gles3', 'api_version': 3.0}]))

    #
    # Test framebuffer fetch functionality with multiple assignments
    # of the fragment color input-output.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= 3.00
        GL_${extension}

        [vertex shader]
        #version 300 es

        in vec4 vertex;
        out vec4 vcolor;

        void main()
        {
            gl_Position = vertex;
            // Transform the vertex coordinates so that the R and G
            // components of the vertex color range between 0.0 and 1.0.
            vcolor = vec4((1.0 + vertex.x) / 2.0,
                          (1.0 + vertex.y) / 2.0, 0.0, 1.0);
        }

        [fragment shader]
        #version 300 es
        #extension GL_${extension} : enable

        in highp vec4 vcolor;
        ${decl_frag_data(api_version, layout)}

        void main()
        {
            // The conditional assignment will be executed for the top
            // and right quadrants.
            if (vcolor.x >= 0.5 || vcolor.y >= 0.5)
                ${frag_data(api_version)} += vec4(0.5, 0, 0, 0);

            ${frag_data(api_version)} += vec4(0.0, 0.5, 0, 0);
        }

        [test]
        ${bind_fb('GL_RGBA8')}

        clear color 0.0 0.0 1.0 0.0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        relative probe rect rgb (0.0, 0.0, 0.45, 0.45) (0.0, 1.0, 1.0)
        relative probe rect rgb (0.55, 0.0, 0.45, 0.45) (1.0, 1.0, 1.0)
        relative probe rect rgb (0.0, 0.55, 0.45, 0.45) (1.0, 1.0, 1.0)
        relative probe rect rgb (0.55, 0.55, 0.45, 0.45) (1.0, 1.0, 1.0)

        ${display_fb(api_version)}
    """, product(all_defs,
                 [{'name': 'overwrite-gles3',
                   'api_version': 3.0}]))

    #
    # Test framebuffer fetch functionality on individual slices of a
    # texture render target with multiple layers or mipmap levels.
    #
    gen_execution("""\
        [require]
        GL ES >= ${api_version}
        GLSL ES >= 3.00
        GL_${extension}

        [vertex shader passthrough]

        [fragment shader]
        #version 300 es
        #extension GL_${extension} : enable

        ${decl_frag_data(api_version, layout)}
        uniform highp vec4 ucolor;

        void main()
        {
            ${frag_data(api_version)} = ${last_frag_data(api_version)} +
                                        ucolor;
        }

        [test]
        %if target == 'Cube':
        texture storage 0 ${target} GL_RGBA8 (${levels} 250 250)
        %else:
        texture storage 0 ${target} GL_RGBA8 (${levels} 250 250 ${layers})
        %endif

        <%! blend_colors = ['0.5 0.0 0.0 0.0',
                            '0.0 0.5 0.0 0.0',
                            '0.5 0.5 0.0 0.0',
                            '0.0 0.0 0.5 0.0'] %>

        %for l in range(0, levels):
        %for z in range(0, layers):
        fb tex slice ${target} 0 ${l} ${z}
        uniform vec4 ucolor ${blend_colors[(l + z) % 4]}
        clear color 0.0 0.0 0.5 0.0
        clear
        ${barrier}
        draw rect -1 -1 2 2
        ${barrier}
        draw rect -1 -1 2 2

        %endfor
        %endfor

        <%! expected_colors = ['(1.0, 0.0, 0.5)',
                               '(0.0, 1.0, 0.5)',
                               '(1.0, 1.0, 0.5)',
                               '(0.0, 0.0, 1.0)'] %>

        %for l in range(0, levels):
        %for z in range(0, layers):
        fb tex slice ${target} 0 ${l} ${z}
        relative probe rect rgb (0.0, 0.0, 1.0, 1.0) ${expected_colors[(l + z) % 4]}

        %endfor
        %endfor

        ${display_fb(api_version)}
    """, product(all_defs,
                 [{'name': 'single-slice-',
                   'api_version': 3.0}],
                 [{'name': '2darray-gles3', 'target': '2DArray',
                   'levels': 1, 'layers': 4},
                  {'name': '2darray-mipmap-gles3', 'target': '2DArray',
                   'levels': 4, 'layers': 1},
                  {'name': '3d-gles3', 'target': '3D',
                   'levels': 1, 'layers': 4},
                  {'name': 'cubemap-gles3', 'target': 'Cube',
                   'levels': 1, 'layers': 6}]))

if __name__ == '__main__':
    main()
