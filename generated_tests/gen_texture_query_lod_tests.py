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
import os.path
from textwrap import dedent

from mako.template import Template

SAMPLER_TYPE_TO_COORD_TYPE = {
    'sampler1D':              'float',
    'isampler1D':             'float',
    'usampler1D':             'float',

    'sampler2D':              'vec2',
    'isampler2D':             'vec2',
    'usampler2D':             'vec2',

    'sampler3D':              'vec3',
    'isampler3D':             'vec3',
    'usampler3D':             'vec3',

    'samplerCube':            'vec3',
    'isamplerCube':           'vec3',
    'usamplerCube':           'vec3',

    'sampler1DArray':         'float',
    'isampler1DArray':        'float',
    'usampler1DArray':        'float',

    'sampler2DArray':         'vec2',
    'isampler2DArray':        'vec2',
    'usampler2DArray':        'vec2',

    'samplerCubeArray':       'vec3',
    'isamplerCubeArray':      'vec3',
    'usamplerCubeArray':      'vec3',

    'sampler1DShadow':        'float',
    'sampler2DShadow':        'vec2',
    'samplerCubeShadow':      'vec3',
    'sampler1DArrayShadow':   'float',
    'sampler2DArrayShadow':   'vec2',
    'samplerCubeArrayShadow': 'vec3',
}

REQUIREMENTS = {
    'ARB_texture_query_lod': {
        'version': '1.30',
        'extension': 'GL_ARB_texture_query_lod'
    },
    'glsl-4.00': {
        'version': '4.00',
        'extension': ''
    }
}

TEMPLATE = Template(dedent("""\
    /* [config]
    % if execution_stage == 'fs':
     * expect_result: pass
    % else:
     * expect_result: fail
    % endif
     * glsl_version: ${version}
    % if extensions:
     * require_extensions: ${" ".join(extensions)}
    % endif
     * [end config]
     */

    #version ${version.translate(None, '.')}
    % for extension in extensions:
    #extension ${extension} : enable
    % endfor

    uniform ${sampler_type} s;
    % if execution_stage == 'fs':
    varying ${coord_type} coord;
    % else:
    uniform ${coord_type} coord;
    % endif

    void main()
    {
        % if execution_stage == 'fs':
        gl_FragColor.xy = textureQuery${Lod}(s, coord);
        % else:
        gl_Position.xy = textureQuery${Lod}(s, coord);
        % endif
    }
"""))

for api, requirement in REQUIREMENTS.iteritems():
    Lod = 'Lod' if api == 'glsl-4.00' else 'LOD'

    for sampler_type, coord_type in SAMPLER_TYPE_TO_COORD_TYPE.iteritems():
        for execution_stage in ("vs", "fs"):
            file_extension = 'frag' if execution_stage == 'fs' else 'vert'
            filename = os.path.join(
                "spec", api.lower(), "compiler", "built-in-functions",
                "textureQuery{0}-{1}.{2}".format(Lod, sampler_type,
                                                 file_extension))
            print(filename)

            dirname = os.path.dirname(filename)
            if not os.path.exists(dirname):
                os.makedirs(dirname)

            version = requirement['version']
            extensions = [requirement['extension']] if requirement['extension'] else []

            # samplerCubeArray types are part GLSL 4.00
            # or GL_ARB_texture_cube_map_array.
            if api == "ARB_texture_query_lod" and sampler_type in [
                    'samplerCubeArray', 'isamplerCubeArray',
                    'usamplerCubeArray', 'samplerCubeArrayShadow']:
                extensions += ['GL_ARB_texture_cube_map_array']

            with open(filename, "w") as f:
                f.write(TEMPLATE.render(version=version,
                                        extensions=extensions,
                                        execution_stage=execution_stage,
                                        sampler_type=sampler_type,
                                        coord_type=coord_type,
                                        Lod=Lod))
