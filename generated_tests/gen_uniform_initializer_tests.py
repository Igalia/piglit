# coding=utf-8
#
# Copyright © 2012, 2014 Intel Corporation
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

TEMPLATES = template_dir(os.path.splitext(os.path.basename(__file__))[0])

# These are a set of pseudo random values used by the number sequence
# generator.  See get_value above.
RANDOM_NUMBERS = (0.78685, 0.89828, 0.36590, 0.92504, 0.48998, 0.27989,
                  0.08693, 0.48144, 0.87644, 0.18080, 0.95147, 0.18892,
                  0.45851, 0.76423, 0.78659, 0.97998, 0.24352, 0.60922,
                  0.45241, 0.33045, 0.27233, 0.92331, 0.63593, 0.67826,
                  0.12195, 0.24853, 0.35977, 0.41759, 0.79119, 0.54281,
                  0.04089, 0.03877, 0.58445, 0.43017, 0.58635, 0.48151,
                  0.58778, 0.37033, 0.47464, 0.17470, 0.18308, 0.49466,
                  0.45838, 0.30337, 0.71273, 0.45083, 0.88339, 0.47350,
                  0.86539, 0.48355, 0.92923, 0.79107, 0.77266, 0.71677,
                  0.79860, 0.95149, 0.05604, 0.16863, 0.14072, 0.29028,
                  0.57637, 0.13572, 0.36011, 0.65431, 0.38951, 0.73245,
                  0.69497, 0.76041, 0.31016, 0.48708, 0.96677, 0.58732,
                  0.33741, 0.73691, 0.24445, 0.35686, 0.72645, 0.65438,
                  0.00824, 0.00923, 0.87650, 0.43315, 0.67256, 0.66939,
                  0.87706, 0.73880, 0.96248, 0.24148, 0.24126, 0.24673,
                  0.18999, 0.10330, 0.78826, 0.23209, 0.59548, 0.23134,
                  0.72414, 0.88036, 0.54498, 0.32668, 0.02967, 0.12643)

ALL_TEMPLATES = ("",
                 "-from-const",
                 "-set-by-API",
                 "-set-by-other-stage")


def get_value(type_, idx):
    """Get a string representing a number in the specified GLSL type"""

    value = RANDOM_NUMBERS[idx % len(RANDOM_NUMBERS)]

    if type_[0] == 'b':
        if (value * 10) > 5:
            return "1"
        else:
            return "0"
    elif type_[0] == 'i':
        return str(int(value * 100) - 50)
    elif type_[0] == 'u':
        return str(int(value * 50))
    else:
        return str((value * 20.0) - 10.0)


def generate_tests(type_list, base_name, major, minor):
    dirname = os.path.join('spec',
                           'glsl-{0}.{1}'.format(major, minor),
                           'execution',
                           'uniform-initializer')
    if not os.path.exists(dirname):
        try:
            os.makedirs(dirname)
        except OSError as e:
            if e.errno == 17:  # file exists
                pass
            raise

    for target in ("vs", "fs"):
        for t in ALL_TEMPLATES:
            template = TEMPLATES.get_template(
                "{0}-initializer{1}.shader_test.mako".format(target, t))

            test_file_name = os.path.join(
                dirname,
                '{0}-{1}{2}.shader_test'.format(target, base_name, t))
            print(test_file_name)

            # Generate the test vectors.  This is a list of tuples.  Each
            # tuple is a type name paired with a value.  The value is
            # formatted as a GLSL constructor.
            #
            # A set of types and values is also generated that can be set via
            # the OpenGL API.  Some of the tests use this information.
            test_vectors = []
            api_vectors = []
            for i, (type_, num_values) in enumerate(type_list):
                numbers = []
                alt_numbers = []
                for j in xrange(num_values):
                    numbers.append(get_value(type_, i + j))
                    alt_numbers.append(get_value(type_, i + j + 7))

                value = "{0}({1})".format(type_, ", ".join(numbers))

                api_type = type_
                if type_ == "bool":
                    api_type = "int"
                elif type_[0] == 'b':
                    api_type = "ivec{0}".format(type_[-1])

                if type_[-1] in ["2", "3", "4"]:
                    name = 'u{0}{1}'.format(type_[0], type_[-1])
                else:
                    name = 'u{0}'.format(type_[0])

                test_vectors.append((type_, name, value))
                api_vectors.append((api_type, name, alt_numbers))

            with open(test_file_name, "w") as f:
                f.write(template.render(type_list=test_vectors,
                                        api_types=api_vectors,
                                        major=major,
                                        minor=minor))


def generate_array_tests(type_list, base_name, major, minor):
    dirname = os.path.join('spec',
                           'glsl-{0}.{1}'.format(major, minor),
                           'execution',
                           'uniform-initializer')
    if not os.path.exists(dirname):
        os.makedirs(dirname)

    def parts():
        """Generate parts."""
        # pylint: disable=undefined-loop-variable
        for j in xrange(2):
            numbers = []
            for k in xrange(num_values):
                numbers.append(get_value(type_, i + j + k))

            yield '{0}({1})'.format(type_, ', '.join(numbers))
        # pylint: enable=undefined-loop-variable

    vecs = []
    for i, (type_, num_values) in enumerate(type_list):
        if type_[-1] in ["2", "3", "4"]:
            name = 'u{0}{1}'.format(type_[0], type_[-1])
        else:
            name = 'u{0}'.format(type_[0])

        array_type = '{0}[2]'.format(type_)
        value = "{0}({1})".format(array_type, ", ".join(parts()))

        vecs.append((array_type, name, value))

    for target in ("vs", "fs"):
        template = TEMPLATES.get_template(
            '{0}-initializer.shader_test.mako'.format(target))

        test_file_name = os.path.join(
            dirname,
            '{0}-{1}-array.shader_test'.format(target, base_name))
        print(test_file_name)

        with open(test_file_name, "w") as f:
            f.write(template.render(type_list=vecs,
                                    major=major,
                                    minor=minor))


def main():
    """Main function."""
    bool_types = [("bool", 1), ("bvec2", 2), ("bvec3", 3), ("bvec4", 4)]
    int_types = [("int", 1), ("ivec2", 2), ("ivec3", 3), ("ivec4", 4)]
    float_types = [("float", 1), ("vec2", 2), ("vec3", 3), ("vec4", 4)]
    uint_types = [("uint", 1), ("uvec2", 2), ("uvec3", 3), ("uvec4", 4)]
    mat2_types = [("mat2x2", 4), ("mat2x3", 6), ("mat2x4", 8)]
    mat3_types = [("mat3x2", 6), ("mat3x3", 9), ("mat3x4", 12)]
    mat4_types = [("mat4x2", 8), ("mat4x3", 12), ("mat4x4", 16)]

    for types, base_name, major, minor in [(bool_types, "bool", 1, 20),
                                           (int_types, "int", 1, 20),
                                           (float_types, "float", 1, 20),
                                           (mat2_types, "mat2", 1, 20),
                                           (mat3_types, "mat3", 1, 20),
                                           (mat4_types, "mat4", 1, 20),
                                           (uint_types, "uint", 1, 30)]:
        generate_tests(types, base_name, major, minor)
        generate_array_tests(types, base_name, major, minor)


if __name__ == '__main__':
    main()
