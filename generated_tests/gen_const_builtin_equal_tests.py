# Copyright (c) 2010, 2014 Intel Corporation

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

""" Generate tests for builtin const equality tests """

from __future__ import print_function, division
import re
import os

from templates import template_file
from modules import utils

TEMPLATE = template_file(os.path.basename(os.path.splitext(__file__)[0]),
                         'template.shader_test.mako')

TEST_VECTORS = [
    ["vec2(3.0, 3.14)",
     "vec2(-6.0, 7.88)",
     "bvec2(false, false)"
    ],
    ["vec3(13.4, -0.9, 12.55)",
     "vec3(13.4, 12.0, -55.3)",
     "bvec3(true, false, false)"
    ],
    ["vec4(-2.0, 0.0, 0.123, -1000.5)",
     "vec4(-2.4, 0.0, 0.456, 12.5)",
     "bvec4(false, true, false, false)"
    ],
    ["ivec2(-8, 12)",
     "ivec2(-19, 12)",
     "bvec2(false, true)"
    ],
    ["ivec3(0, 8, 89)",
     "ivec3(4, -7, 33)",
     "bvec3(false, false, false)"
    ],
    ["ivec4(11, 1000, 1, -18)",
     "ivec4(55, 1000, -21, -17)",
     "bvec4(false, true, false, false)"
    ],
    ["bvec2(true, false)",
     "bvec2(true, true)",
     "bvec2(true, false)"
    ],
    ["bvec3(false, true, false)",
     "bvec3(false, false, true)",
     "bvec3(true, false, false)"
    ],
    ["bvec4(true, false, false, true)",
     "bvec4(true, true, false, false)",
     "bvec4(true, false, true, false)"
    ],
]


def main():
    """ Main function """
    dirname = os.path.join('spec', 'glsl-1.20', 'execution',
                           'built-in-functions')
    utils.safe_makedirs(dirname)

    for test_id, x in enumerate(TEST_VECTORS, start=2):
        # make equal tests
        name = os.path.join(
            dirname,
            "glsl-const-builtin-equal-{0:02d}.shader_test".format(test_id))

        print(name)

        with open(name, 'w') as f:
            f.write(TEMPLATE.render_unicode(
                func='equal', input=x[0:2], expected=x[2]))

        # make notEqual tests
        name = os.path.join(
            dirname,
            "glsl-const-builtin-notEqual-{0:02d}.shader_test".format(test_id))

        # When generating the notEqual tests, each of the values in the
        # expected result vector need to be inverted
        expected = re.sub("true", "FALSE", x[2])
        expected = re.sub("false", "TRUE", expected)
        expected = expected.lower()

        print(name)

        with open(name, 'w') as f:
            f.write(TEMPLATE.render_unicode(
                func='notEqual', input=x[0:2], expected=expected))


if __name__ == "__main__":
    main()
