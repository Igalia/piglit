#!/usr/bin/env python2
# coding=utf-8

# Copyright (c) 2019 Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

""" Generate in/out tests for 64 bit types. """

import os
import errno
import collections
import copy
import random_ubo

from six.moves import range
from textwrap import dedent
from mako.template import Template


class VaryingType(object):
    __slots__ = ['type', 'members', 'name']

    def __init__(self, type, members=None):
        self.type = type
        self.members = members
        self.name = None


VaryingInstance = collections.namedtuple(
    'VaryingInstance', ['varying_type', 'setters', 'checkers', 'full_name'])


def scalar_assignment(type, name, data):
    """Return a GLSL code string to assign a scalar to its expected value."""
    if type == "bool":
        if int(data) == 0:
            return "{} = {};".format(name,
                                     'false' if int(data) == 0 else 'true')
    elif type == "uint":
        return "{} = {}u;".format(name, data)
    elif type == "int":
        return "{} = {};".format(name, data)
    elif type == "uint64_t":
        return "{} = {}ul;".format(name, data)
    elif type == "int64_t":
        return "{} = {}l;".format(name, data)
    elif type == "float":
        # Not all implementations support the bit-cast operators that are used
        # to do bit-exact comparisons.  For this reason float_match needs the
        # float data and the bit-exact data.

        bits = random_ubo.bit_exact_data(data, "float")
        return "{} = float_get({}, {}u);".format(name, data, bits)
    elif type == "double":
        bits = random_ubo.bit_exact_data(data, "double")

        # 0xHHHHHHHHLLLLLLLL
        # 012345678901234567

        hi = "0x" + bits[2:10]
        lo = "0x" + bits[10:18]

        return "{} = double_get(uvec2({}, {}));".format(name, lo, hi)
    else:
        raise Exception("Unknown scalar type {}".format(type))


def vector_assignment(type, name, data):
    """Return a list of GLSL code strings that assign each field of a vector
    to its expected value.
    """
    scalar = random_ubo.vector_base_type(type)
    components = ["x", "y", "z", "w"]

    return [scalar_assignment(scalar,
                              "{}.{}".format(name, "xyzw"[i]),
                              data[i])
            for i in range(random_ubo.vector_size(type))]


def matrix_assignment(type, name, data):
    """Return a list of GLSL code strings that assign each field of a matrix
    its expected value.
    """
    c, r = random_ubo.matrix_dimensions(type)

    if type[0] == 'd':
        column_type = "dvec{}".format(r)
    else:
        column_type = "vec{}".format(r)

    data_pairs = []

    for i in range(c):
        data_pairs.extend(vector_assignment(
            column_type,
            "{}[{}]".format(name, i),
            data[(i * r):(i * r) + r]))

    return data_pairs


def create_array(base_type, size):
    return "{}[{}]".format(base_type, size)


def array_base_type(varying_type):
    t = copy.copy(varying_type)
    t.type = random_ubo.array_base_type(t.type)
    t.name = None
    return t


def create_getters_and_setters(varying_type, full_name, instances):
    if not full_name:
        full_name = varying_type.name
    elif varying_type.name is not None:
        full_name += '.' + varying_type.name

    if random_ubo.isarray(varying_type.type):
        base_type = array_base_type(varying_type)
        for i in range(random_ubo.array_elements(varying_type.type)):
            indexed_name = full_name + '[' + str(i) + ']'
            create_getters_and_setters(base_type, indexed_name, instances)

    elif random_ubo.isstructure(varying_type.type):
        for m in varying_type.members:
            create_getters_and_setters(m, full_name, instances)

    else:
        v = VaryingInstance(varying_type, [], [], full_name)

        raw_data = random_ubo.random_data(varying_type.type, full_name, 0)
        data = raw_data.split(" ")

        if random_ubo.isscalar(varying_type.type):
            v.checkers.append(random_ubo.scalar_derp(varying_type.type,
                                                     full_name,
                                                     data[0]))
            v.setters.append(
                scalar_assignment(varying_type.type, full_name, data[0]))
        elif random_ubo.isvector(varying_type.type):
            v.checkers.extend(random_ubo.vector_derp(varying_type.type,
                                                     full_name,
                                                     data))
            v.setters.extend(
                vector_assignment(varying_type.type, full_name, data))
        elif random_ubo.ismatrix(varying_type.type):
            v.checkers.extend(random_ubo.matrix_derp(varying_type.type,
                                                     full_name,
                                                     data))
            v.setters.extend(
                matrix_assignment(varying_type.type, full_name, data))

        instances.append(v)


def assign_names(varying_type, names):
    if random_ubo.isarray(varying_type.type):
        varying_type.name = names.get_name(
            random_ubo.without_array(varying_type.type))
    else:
        varying_type.name = names.get_name(varying_type.type)

    if varying_type.members:
        for m in varying_type.members:
            assign_names(m, names)


def gather_structs(varying_type, structs):
    if random_ubo.isarray(varying_type.type):
        base_type = array_base_type(varying_type)
        gather_structs(base_type, structs)
    elif random_ubo.isstructure(varying_type.type):
        for m in varying_type.members:
            gather_structs(m, structs)

        structs.append(varying_type)


def stringify_array_dimensions(type):
    s = ""
    base_type = type
    while random_ubo.isarray(base_type):
        s += "@{}".format(random_ubo.array_elements(base_type))
        base_type = random_ubo.array_base_type(base_type)
    return s


def stringify_varying_type(varying_type):
    if random_ubo.isarray(varying_type.type):
        name = "{}{}".format(random_ubo.without_array(varying_type.type),
                             stringify_array_dimensions(varying_type.type))
    else:
        name = varying_type.type

    if varying_type.members:
        for m in varying_type.members:
            name += "-" + stringify_varying_type(m)

    return name


def generate_file_name(root_types, explicit_locations):
    name = "vs-out-fs-in-"
    name += "-and-".join(
        [stringify_varying_type(t) for t in root_types])
    if explicit_locations:
        name += "-location-0"
    return name + ".shader_test"


def do_test(root_types, explicit_locations, glsl_version, extensions,
            tests_path):
    if explicit_locations and len(root_types) > 1:
        return

    basename = generate_file_name(root_types, explicit_locations)
    fullname = os.path.join(tests_path, basename)

    print(fullname)

    shader_file = open(fullname, "w", buffering=1)

    names = random_ubo.unique_name_dict()

    instances = []
    structs = []

    for type in root_types:
        assign_names(type, names)
        create_getters_and_setters(type, None, instances)
        gather_structs(type, structs)

    t = Template(dedent("""\
    [require]
    GLSL >= ${glsl_version // 100}.${glsl_version % 100}
    % for ext in extensions:
    ${ext}
    % endfor

    % if explicit_locations:
    GL_ARB_explicit_attrib_location
    % endif

    [vertex shader]
    % for ext in extensions:
    #extension ${ext}: require
    % endfor
    #extension GL_ARB_shader_bit_encoding: enable
    #extension GL_ARB_gpu_shader5: enable

    % if explicit_locations:
    #extension GL_ARB_explicit_attrib_location : require
    % endif

    precision highp float;
    % for s in structures:

    struct ${s.type} {
        % for m in s.members:
        ${"{:<15}".format(m.type)} ${m.name};
        % endfor
    };
    % endfor

    % for r in root_types:
    % if explicit_locations:
    layout(location = 0)
    % endif
    flat out ${"{:<10}".format(r.type)} ${r.name};
    % endfor

    in vec4 piglit_vertex;

    #if defined(GL_ARB_shader_bit_encoding) || defined(GL_ARB_gpu_shader5) || __VERSION__ >= 430
    float float_get(float f, uint bits) { return uintBitsToFloat(bits); }
    #else
    float float_get(float f, uint bits) { return f; }
    #endif
    % if glsl_version >= 400 or "GL_ARB_gpu_shader_fp64" in extensions:
    double double_get(uvec2 bits) { return packDouble2x32(bits); }
    %endif

    void main()
    {
    % for inst in instances:
        % for s in inst.setters:
        ${s}
        % endfor
    % endfor

        gl_Position = piglit_vertex;
    }

    [fragment shader]
    % for ext in extensions:
    #extension ${ext}: require
    % endfor
    #extension GL_ARB_shader_bit_encoding: enable
    #extension GL_ARB_gpu_shader5: enable

    % if explicit_locations:
    #extension GL_ARB_explicit_attrib_location : require
    % endif

    precision highp float;

    % for s in structures:
    struct ${s.type} {
        % for m in s.members:
        ${"{:<15}".format(m.type)} ${m.name};
        % endfor
    };
    % endfor

    % for r in root_types:
    % if explicit_locations:
    layout(location = 0)
    % endif
    flat in ${"{:<11}".format(r.type)} ${r.name};
    % endfor

    out vec4 piglit_fragcolor;

    #if defined(GL_ARB_shader_bit_encoding) || defined(GL_ARB_gpu_shader5) || __VERSION__ >= 430
    bool float_match(float u, float f, uint bits) { return floatBitsToUint(u) == bits; }
    #else
    bool float_match(float u, float f, uint bits) { return u == f; }
    #endif
    % if glsl_version >= 400 or "GL_ARB_gpu_shader_fp64" in extensions:
    bool double_match(double u, uvec2 bits) { return unpackDouble2x32(u) == bits; }
    %endif

    void main()
    {
        bool pass = true;

    % for inst in instances:
        % for s in inst.checkers:
        if (${s})
            pass = false;
        % endfor
    % endfor

        piglit_fragcolor = pass ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
    }

    [test]
    link success

    draw rect -1 -1 2 2
    probe all rgba 0.0 1.0 0.0 1.0"""))

    shader = t.render(glsl_version=glsl_version,
                      root_types=root_types,
                      explicit_locations=explicit_locations,
                      structures=structs,
                      extensions=extensions,
                      instances=instances)

    shader_file.write(shader)
    shader_file.close()


def do_test_permutations(root_types, glsl_version, extensions, tests_path):
    without_outer_struct = []
    for type in root_types:
        if random_ubo.isstructure(type.type):
            without_outer_struct.extend(m for m in type.members)
        else:
            without_outer_struct.append(type)

    do_test(root_types, False, glsl_version, extensions, tests_path)
    do_test(root_types, True, glsl_version,
            extensions + ["GL_ARB_separate_shader_objects"], tests_path)
    do_test(without_outer_struct, False, glsl_version, extensions, tests_path)


fp64_ext_path = os.path.join("spec", "arb_gpu_shader_fp64", "execution",
                             "inout")
fp64_core_path = os.path.join("spec", "glsl-4.00", "execution",
                              "inout")
int64_path = os.path.join("spec", "arb_gpu_shader_int64", "execution",
                          "inout")

for path in [fp64_ext_path, fp64_core_path, int64_path]:
    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno != errno.EEXIST or not os.path.isdir(path):
            raise

INT64_VEC_TYPES = [None, "int64_t", "i64vec2", "i64vec3", "i64vec4"]
UINT64_VEC_TYPES = [None, "uint64_t", "u64vec2", "u64vec3", "u64vec4"]
FLOAT_VEC_TYPES = [None, "float", "vec2", "vec3", "vec4"]
DOUBLE_VEC_TYPES = [None, "double", "dvec2", "dvec3", "dvec4"]

DOUBLE_MAT_TYPES = ["dmat2x2", "dmat2x3", "dmat2x4", "dmat3x2", "dmat3x3",
                    "dmat3x4", "dmat4x2", "dmat4x3", "dmat4x4"]

VT = VaryingType


def do_common_64bit_tests(glsl_version, extensions, name_arr, tests_path):
    tests = []
    for i in range(1, 4):
        tests.append([VT("S1", [VT(name_arr[i])])])

        tests.append([VT("S1", [VT(name_arr[i] + "[4]")])])

        for j in range(1, 4):
            tests.append([VT("S1", [VT(FLOAT_VEC_TYPES[j]), VT(name_arr[i])])])

        tests.append([VaryingType("S1",
                                  [VT("float"), VT("float"), VT("float"),
                                   VT(name_arr[i])])])

        for j in range(2, 5):
            tests.append([VT("S1",
                             [VT("float[{}]".format(j)),
                              VT(name_arr[i])])])

        tests.append([VT("S1", [VT(name_arr[i] + "[3]")])])

        tests.append([VT("S1",
                         [VT("S2[3]", [VT(name_arr[i]), VT("float")])])])

        tests.append([VT("S1",
                         [VT("S2", [VT(name_arr[i])])])])

    for i in range(1, 2):
        tests.append([VT("S1",
                         [VT("S2[2]",
                             [VT("S3[2]", [
                                 VT("float"),
                                 VT(name_arr[i])])])])])

        tests.append([VT("S1",
                         [VT("S2[2]",
                             [VT("S3[2]", [
                                 VT("vec3"),
                                 VT(name_arr[i])])])])])

        tests.append([VT("S1[2]",
                         [VT("S2[2]",
                             [VT("S3[2]", [
                                 VT(name_arr[i])])])])])

    for t in tests:
        do_test_permutations(t, glsl_version, extensions, tests_path)

    tests = []

    for i in range(1, 2):
        tests.append([VT("S1",
                         [VT(name_arr[i] + "[3][2]")])])
    for i in range(1, 2):
        for j in range(1, 4):
            tests.append([VT("S1",
                             [VT(FLOAT_VEC_TYPES[j]),
                              VT(name_arr[i] + "[3][2]")])])

    for i in range(1, 2):
        for j in range(1, 4):
            tests.append([VT("S1", [VT("S2[2][2]",
                                       [VT(FLOAT_VEC_TYPES[j]),
                                        VT(name_arr[i])])])])

    for i in range(3, 4):
        tests.append([VT("S1", [VT(name_arr[i] + "[2][2]")])])

    for t in tests:
        do_test_permutations(t, glsl_version,
                             extensions + ["GL_ARB_arrays_of_arrays"],
                             tests_path)


def do_fp64_specific_tests():
    tests = []

    for t in DOUBLE_MAT_TYPES:
        tests.append([VT("S1", [VT(t)])])

    for t in ["dmat2x2", "dmat2x3", "dmat2x4", "dmat3x2", "dmat3x3"]:
        for j in range(1, 4):
            tests.append([VT("S1", [VT(FLOAT_VEC_TYPES[j]), VT(t)])])

        for j in range(1, 7):
            tests.append([VT("S1",
                             [VT("float[{}]".format(j)), VT(t)])])

    for j in range(1, 4):
        tests.append([VT("S1", [VT("S2[2]",
                                   [VT(FLOAT_VEC_TYPES[j]),
                                    VT("dmat2x2")])])])

    tests.append([VT("S1",
                     [VT("double"), VT("float"), VT("double[2]"),
                      VT("float[3]"), VT("dmat2x2")])])

    tests.append([VT("S1",
                     [VT("S2", [VT("double")]),
                      VT("S3", [VT("float")]),
                      VT("S4", [VT("dmat3x3")])])])

    for t in tests:
        do_test_permutations(t, 150, ["GL_ARB_gpu_shader_fp64"], fp64_ext_path)
        do_test_permutations(t, 400, [], fp64_core_path)


do_common_64bit_tests(400, [], DOUBLE_VEC_TYPES, fp64_core_path)
do_common_64bit_tests(150, ["GL_ARB_gpu_shader_fp64"], DOUBLE_VEC_TYPES,
                      fp64_ext_path)
do_common_64bit_tests(150, ["GL_ARB_gpu_shader_int64"], INT64_VEC_TYPES,
                      int64_path)
do_common_64bit_tests(150, ["GL_ARB_gpu_shader_int64"], UINT64_VEC_TYPES,
                      int64_path)

do_fp64_specific_tests()
