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

"""Generate a set of tests to verify that layout qualifiers introduced
 by INTEL_conservative_rasterization are properly parsed.

 This also verifies the interaction with ARB_post_depth_coverage.

 This program outputs, to stdout, the name of each file it generates.

"""

import os.path
from textwrap import dedent

from mako.template import Template

from modules import utils

def gen_header(status, gl_api, shader_stage):
    """
    Generate a GLSL program header.

    Generate header code for INTEL_conservative_rasterization GLSL parser
    tests that are expected to give status as result.
    """

    if shader_stage != 'frag':
        status = 'fail'
    print("%s - %s" % (shader_stage, status))

    if gl_api == "gles":
        glsl_version = ("3.20 es", "320 es")
    else:
        glsl_version = ("4.20", "420")

    return dedent("""
        /*
         * [config]
         * expect_result: {0}
         * glsl_version: {1}
         * require_extensions: GL_INTEL_conservative_rasterization
         * [end config]
         */
        #version {2}
        #extension GL_INTEL_conservative_rasterization : enable
    """.format(status, glsl_version[0], glsl_version[1]))


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
                                'intel_conservative_rasterization',
                                'compiler',
                                '{0}.{1}.{2}'.format(t['name'],
                                                     t['gl_api'],
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


gl_apis = [
    {'gl_api': 'gl'},
    {'gl_api': 'gles'}
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
    # Test inner_coverage layout qualifier.
    #
    gen('inner_coverage', """
        ${header('pass', gl_api, shader_stage)}

        layout(inner_coverage) in;

        void main()
        {
        }
    """, product(gl_apis, shader_stages))

    #
    # Test depth_coverage layout qualifier.
    #
    gen('post_depth_coverage', """
        ${header('pass', gl_api, shader_stage)}

        layout(post_depth_coverage) in;

        void main()
        {
        }
    """, product(gl_apis, shader_stages))

    #
    # Test depth_coverage layout qualifier.
    #
    gen('inner_post_depth_coverage', """
        ${header('fail', gl_api, shader_stage)}

        layout(inner_coverage) in;
        layout(post_depth_coverage) in;

        void main()
        {
        }
    """, product(gl_apis, shader_stages))

if __name__ == '__main__':
    main()
