#!/usr/bin/env python2
# coding=utf-8

# Copyright (c) 2014 Intel Corporation
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

from __future__ import print_function
import random
import abc
import collections
import struct
import sys
from textwrap import dedent
from mako.template import Template

# Module-local global variable that tracks all of the structure types that
# have been generated since ever.
struct_types = dict()

ALL130_TYPES = [
    "float",  "vec2",  "vec3",  "vec4",
    "int",    "ivec2", "ivec3", "ivec4",
    "uint",   "uvec2", "uvec3", "uvec4",
    "bool",   "bvec2", "bvec3", "bvec4",

    "mat2",    "mat2x3",  "mat2x4",
    "mat3x2",  "mat3",    "mat3x4",
    "mat4x2",  "mat4x3",  "mat4"
]

DOUBLE_TYPES = [
    "double", "dvec2", "dvec3", "dvec4",

    "dmat2",   "dmat2x3", "dmat2x4",
    "dmat3x2", "dmat3",   "dmat3x4",
    "dmat4x2", "dmat4x3", "dmat4"
]

ALL400_TYPES = ALL130_TYPES + DOUBLE_TYPES

# All known types, including the redundant NxN matrix types.
ALL_TYPES = [ "mat2x2",  "mat3x3",  "mat4x4",
             "dmat2x2", "dmat3x3", "dmat4x4"] + ALL400_TYPES

def align(offset, alignment):
    return ((offset + alignment - 1) / alignment) * alignment


def isscalar(type):
    """Return true if the type is a known scalar type from any GLSL version."""
    return type in ["float", "bool", "int", "uint", "double"]


def isvector(type):
    """Return true if the type is a known vector type from any GLSL version."""
    return type in ["vec2",  "vec3",  "vec4",
                    "ivec2", "ivec3", "ivec4",
                    "uvec2", "uvec3", "uvec4",
                    "bvec2", "bvec3", "bvec4",
                    "dvec2", "dvec3", "dvec4"]


def ismatrix(type):
    """Return true if the type is a known matrix type from any GLSL version."""
    return type in ["mat2",    "mat3",    "mat4",
                    "mat2x2",  "mat2x3",  "mat2x4",
                    "mat3x2",  "mat3x3",  "mat3x4",
                    "mat4x2",  "mat4x3",  "mat4x4",
                    "dmat2",   "dmat3",   "dmat4",
                    "dmat2x2", "dmat2x3", "dmat2x4",
                    "dmat3x2", "dmat3x3", "dmat3x4",
                    "dmat4x2", "dmat4x3", "dmat4x4"]


def isarray(type):
    """Return true if the type name appears to be an array type."""
    return "[" in type


def isstructure(type):
    """Return true if the type name appears to be a structure type.

    This function works by process of elimination.  If the named type is not a
    scalar, vector, matrix, or array, then it must be a structure.

    NOTE: If the type is an array-of-structure type, this function will return
    false.
    """
    return not (isscalar(type) or isvector(type) or ismatrix(type) or
                isarray(type))


def vector_size(type):
    """Return the number of elements in a vector type."""
    if isvector(type):
        return int(type[-1:])

    raise Exception("Non-vector type {}".format(type))


def matrix_dimensions(type):
    """Return the dimensions of an array type.

    The tuple returned is (columns, rows).
    """
    if not ismatrix(type):
        raise Exception("Non-matrix type {}".format(type))

    if "x" in type:
        s = type[-3:].split("x")
        return int(s[0]), int(s[1])
    else:
        d = int(type[-1:])
        return d, d


def array_elements(type):
    """Return the number of elements in an array type.

    For non-array types zero is returned.
    """
    if "[" not in type:
        return 0

    # Is there a better way to do this?
    return int(type.split("[")[1].split("]")[0])

def basic_machine_units(type):
    """Return the size in 'basic machine units' of a scalar type."""
    if type in ["float", "bool", "int", "uint"]:
        return 4

    if type == "double":
        return 8

    raise Exception("Non-scalar type {}".format(type))


def array_base_type(type):
    """Return the type of an element from an array type."""
    if not isarray(type):
        raise Exception("Non-array type {}".format(type))

    return type[:type.index("[")] + type[type.index("]")+1:]

def without_array(type):
    """Return the type of an element with all arrays removed."""
    if not isarray(type):
        raise Exception("Non-array type {}".format(type))

    return type.split("[")[0]

def arrays_of_arrays_size(type):
    """Return the total number of elements in an array of arrays."""
    if not isarray(type):
        raise Exception("Non-array type {}".format(type))

    size = 1
    base_type = type
    while (isarray(base_type)):
        size = size * array_elements(base_type)
        base_type = array_base_type(base_type)

    return size

def vector_base_type(type):
    """Return the type of an element from a vector."""
    if not isvector(type):
        raise Exception("Non-vector type {}".format(type))

    if type[0] == 'v':
        return "float"
    elif type[0] == 'i':
        return "int"
    elif type[0] == 'u':
        return "uint"
    elif type[0] == 'b':
        return "bool"
    elif type[0] == 'd':
        return "double"
    else:
        raise Exception("Unknown vector type {}".format(type))


def iterate_structures(fields, types_seen=[], types_yielded=[]):
    """Given a list of fields, yields the structures in the fields in proper
    declaration order.  Detects recurrsion in the types and raises an
    exception.
    """
    for (type, name) in fields:
        if isarray(type):
            type = without_array(type)

        if not isstructure(type):
            continue

        if type in types_seen:
            raise Exception("Type recurrsion involving {}".format(type))

        for t in iterate_structures(struct_types[type],
                                    types_seen + [type],
                                    types_yielded):
            yield t

        if type not in types_yielded:
            types_yielded.append(type)
            yield type


def select_basic_type(types, names):
    """Return a random type with a matching field name.

    From the list of supplied types, picks a random type.  Using supplied
    unique_name_dict, a name is also chosen.  The two are returned as a tuple.
    """
    t = random.choice(types)
    return t, names.get_name(t)


def generate_struct_of_basic_types(types, names):
    """Return a sequence of random types with unique field names."""
    return [select_basic_type(types, names)
            for i in xrange(0, random.randint(1,12))]


def generate_member_from_description(description, builtin_types, names):
    """Generate a single block member for a list of required attributes.

    The list of required attributes is a sequence of 'struct' and 'array'
    perhaps terminated with the name of a GLSL basic type.  In addition to any
    required subfields, structures generated in the sequence will be populated
    with a random selection of available GLSL types named in builtin_types.

    A matrix orientation can also be listed as the first requirement.
    However, that is not handled by this function, and it will be ignored.

    The tuple returned is (member_type, member_name).
    """
    if len(description) == 0:
        return select_basic_type(builtin_types, names)

    item = description[0]
    if item == "array":
        base_type, name = generate_member_from_description(
            description[1:],
            builtin_types,
            names)

        # If we're making an array of something that can be "big," try to make
        # the array a little smaller.

        if ismatrix(base_type) or isarray(base_type) or isstructure(base_type):
            size = random.choice([2, 3, 5, 7])
        else:
            size = random.choice([3, 5, 7, 11, 13])

        t = "{}[{}]".format(base_type, size)
        return t, name
    elif item == "struct":
        fields = generate_struct_of_basic_types(builtin_types, names)
        random.shuffle(fields)

        # Peek ahead.  If the next item in the description is a built-in type,
        # then all of the remaining items must be built-in types.  Generate a
        # list of these.

        if len(description) > 1 and description[1] in ALL_TYPES:
            required_fields = [generate_member_from_description([i],
                                                                builtin_types,
                                                                names)
                               for i in description[1:]]

        else:
            required_fields = [generate_member_from_description(
                    description[1:],
                    builtin_types,
                    names)]

        # Pick a random spot in the list of "common" fields and insert all of
        # the required fields there.

        j = random.randint(0, len(fields))
        f = fields[:j] + required_fields + fields[j:]

        struct_name = "S{}".format(len(struct_types) + 1)
        struct_types[struct_name] = f

        field_name = names.get_name(struct_name)
        return struct_name, field_name
    elif item in ALL_TYPES:
        return item, names.get_name(item)
    elif item in ["row_major", "column_major", "#column_major"]:
        # While "row_major" and "column_major" are valid requirements, they
        # are not processed here.  Just skip over them for now.
        return generate_member_from_description(description[1:],
                                                builtin_types,
                                                names)

    raise Exception("Invalid UBO member description {}".format(item))


def generate_ubo(description_list, builtin_types):
    """Generate a uniform block for a list of requirements.

    Each member of description_list is taken to be a sequence of requirements
    for a single field in the uniform block.  These are processed by
    generate_member_from_description.

    In addition to the list of required fields, the uniform block is also
    populated with a random selection of available GLSL types named in
    builtin_types.
    """
    layouts = dict()
    names = unique_name_dict()

    fields = []

    for desc in description_list:
        m = generate_member_from_description(desc, builtin_types, names)
        fields.append(m)

        if desc[0] in ["row_major", "column_major", "#column_major"]:
            layouts[m[1]] = desc[0]

    fields.extend(generate_struct_of_basic_types(builtin_types, names))
    random.shuffle(fields)

    required_layouts = []
    for (field_type, field_name) in fields:
        if field_name in layouts:
            required_layouts.append(layouts[field_name])
        else:
            required_layouts.append(None)

    return fields, required_layouts


def generate_layouts(fields, required_layouts, allow_row_major_structure):
    """Generate a directed random sequence of layout qualifiers.

    Elements in requred_layouts that are non-None will have the same value in
    the final sequence.  Remaining elements matching entries in fields that
    are either structure or matrix types will have random layouts assigned.

    If allow_row_major_structure is not set, only matrix types will have
    random layout assignments.  Explicit layouts in required_layouts are
    unaffected.
    """
    if required_layouts is None:
        required_layouts = [None] * len(fields)

    layouts = []
    for ((type, name), lay) in zip(fields, required_layouts):
        if isarray(type):
            type = without_array(type)

        if lay:
            layouts.append(lay)
        elif isstructure(type) and not allow_row_major_structure:
            # This would work-around a bug in NVIDIA closed source drivers.
            # They do not propagate row-major down into structures.

            layouts.append("#column_major")
        elif ismatrix(type) or isstructure(type):
            # Choose a random matrix orientation.  The #column_major are
            # ignored when the UBO is emitted, but when a the UBO is
            # re-emitted with a default row-major layout, these become
            # "column_major".

            layouts.append(random.choice(["#column_major",
                                          "#column_major",
                                          "#column_major",
                                          "row_major",
                                          "row_major",
                                          "column_major"]))
        else:
            layouts.append("")
    return layouts


def relayout_for_default_row_major(l):
    """Return a new layout for an assumed default layout of row-major."""
    if l == "row_major":
        return "#row_major"
    elif l == "column_major" or l == "#column_major":
        return "column_major"
    elif l == "":
        return ""
    else:
        raise Exception("Invalid layout {}".format(l))


def fields_to_glsl_struct(type):
    """From a name structure type, generate a GLSL definition of that
    structure.
    """
    # The longest type name will have the form 'dmatCxR[##]' for 11
    # characters.  Use this to set the spacing between the field type and the
    # field name.

    structure_template = Template(dedent("""\
    struct ${struct_name} {
    % for (field_type, field_name) in fields:
        ${"{:<11}".format(field_type)} ${field_name};
    % endfor
    };
    """))

    return structure_template.render(struct_name=type, fields=struct_types[type])

def iterate_struct_array_recursive(field_type,
                                   name_from_API,
                                   name_from_shader,
                                   packing,
                                   offset,
                                   row_major,
                                   field_row_major,
                                   explicit_layout):

    astride = packing.array_stride(field_type, field_row_major)
    array_member_align = packing.base_alignment(field_type, field_row_major)

    element_t = array_base_type(field_type)

    for i in xrange(array_elements(field_type)):
        name_from_API_with_index = "{}[{}]".format(name_from_API, i)
        name_from_shader_with_index = "{}[{}]".format(name_from_shader, i)

        if isarray(element_t):
            inner_aoa_size = arrays_of_arrays_size(element_t)
            o = align(offset, array_member_align) + (astride * i * inner_aoa_size)

            for x in iterate_struct_array_recursive(element_t,
                                                    name_from_API_with_index,
                                                    name_from_shader_with_index,
                                                    packing,
                                                    o,
                                                    row_major,
                                                    field_row_major,
                                                    explicit_layout):
                yield x

                a = packing.base_alignment(x.GLSL_type, row_major)
                o = align(o, a) + packing.size(x.GLSL_type, row_major)
        else:
            o = align(offset, array_member_align) + (astride * i)
            yield block_member(
                name_from_shader_with_index,
                name_from_API_with_index,
                element_t,
                explicit_layout,
                o,
                field_row_major)
            for x in iterate_all_recursive(struct_types[element_t],
                                           None,
                                           name_from_API_with_index,
                                           name_from_shader_with_index,
                                           packing,
                                           o,
                                           field_row_major):
                yield x

                a = packing.base_alignment(x.GLSL_type, row_major)
                o = align(o, a) + packing.size(x.GLSL_type, row_major)

def iterate_all_recursive(fields,
                          field_layouts,
                          name_from_API_base,
                          name_from_shader_base,
                          packing,
                          offset,
                          row_major):
    """In-order recursive iteration of all subfields of a structure type

    fields -- List of fields in the uniform block or structure
    field_layouts -- Layout for each field in the uniform block (None for
        structures).
    name_from_API_base -- Base name of the member as it would be accessed via
        the GL API.
    name_from_shader_base -- Base name of the member as it would be accessed via
        the GLSL shader.
    packing -- Packing rules used to layout the type
    offset -- Aligned base offset of the member
    row_major -- True if the current matrix orientation is row-major

    At each step of the iteration will yield a block_member object
    representing the member.

    Basic GLSL types will be visited once.  Arrays of basic GLSL types will
    also be visited once.

    A structure will be visited once, then each field in the structure will be
    visited.

    An array of structures will be visited once, then each element of the
    array will be visited.  Since each element is a structure, each field will
    be visited.

    Consider the following structure:

        struct S2 {
            int i;
            float f[2];
        };

        struct S1 {
            S2 s[2];
        };

    If name_from_API_base is "x", the members would be visited in the
    following order: "x.s", "x.s[0]", "x.s[0].i", "x.s[0].f[0]", "x.s[1]",
    "x.s[1].i", "x.s[1].f[0]".

    See also iterate_all_block_members.
    """
    if field_layouts is None:
        field_layouts = [""] * len(fields)

    if len(name_from_shader_base) > 0:
        fmt = "{base}.{field}"
    else:
        fmt = "{field}"

    for ((field_type, field_name), explicit_layout) in zip(fields,
                                                           field_layouts):
        name_from_shader = fmt.format(
            base=name_from_shader_base,
            field=field_name)
        name_from_API = fmt.format(
            base=name_from_API_base,
            field=field_name)

        if explicit_layout == "row_major":
            field_row_major = True
        elif explicit_layout == "column_major":
            field_row_major = False
        else:
            field_row_major = row_major

        if isarray(field_type):
            base_type = without_array(field_type)

            if isstructure(base_type):
                yield block_member(
                    name_from_shader,
                    name_from_API,
                    field_type,
                    explicit_layout,
                    offset,
                    field_row_major)

                for x in iterate_struct_array_recursive(field_type,
                                           name_from_API,
                                           name_from_shader,
                                           packing,
                                           offset,
                                           row_major,
                                           field_row_major,
                                           explicit_layout):
                    yield x

            elif ismatrix(base_type):
                yield block_member(
                    name_from_shader,
                    name_from_API,
                    field_type,
                    explicit_layout,
                    offset,
                    field_row_major)
            else:
                yield block_member(
                    name_from_shader,
                    name_from_API,
                    field_type,
                    "",
                    offset,
                    False)
        elif isstructure(field_type):
            yield block_member(
                name_from_shader,
                name_from_API,
                field_type,
                explicit_layout,
                offset,
                field_row_major)

            a = packing.base_alignment(field_type, field_row_major)

            for x in iterate_all_recursive(struct_types[field_type],
                                           None,
                                           name_from_API,
                                           name_from_shader,
                                           packing,
                                           align(offset, a),
                                           field_row_major):
                yield x

        elif ismatrix(field_type):
            yield block_member(
                name_from_shader,
                name_from_API,
                field_type,
                explicit_layout,
                offset,
                field_row_major)
        elif isvector(field_type) or isscalar(field_type):
            yield block_member(
                name_from_shader,
                name_from_API,
                field_type,
                "",
                offset,
                False)
        else:
            raise Exception("Malformed type name {}".format(field_type))

        a = packing.base_alignment(field_type, field_row_major)
        offset = align(offset, a) + packing.size(field_type, field_row_major)


def iterate_all_block_members(fields,
                              field_layouts,
                              block_name,
                              instance_name,
                              packing,
                              row_major):
    """In-order recursive iteration of all subfields of a block

    fields -- List of fields in the uniform block
    field_layouts -- Layout for each field in the uniform block
    block_name - Name of the uniform block
    instance_name - Instance name associated with the block.  May be None.
    packing -- Packing rules used to layout the type
    row_major -- True if the default block matrix orientation is row-major

    At each step of the iteration will yield a block_member object
    representing the member.

    Basic GLSL types will be visited once.  Arrays of basic GLSL types will
    also be visited once.

    A structure will be visited once, then each field in the structure will be
    visited.

    An array of structures will be visited once, then each element of the
    array will be visited.  Since each element is a structure, each field will
    be visited.

    Consider the following block:

        struct S {
            int i;
            float f[2];
        };

        uniform U {
            S s[2];
        };

    The members would be visited in the following order: s, s[0], s[0].i,
    s[0].f[0], s[1], s[1].i, s[1].f[0].
    """
    for x in iterate_all_recursive(fields,
                                   field_layouts,
                                   block_name,
                                   instance_name,
                                   packing,
                                   0,
                                   row_major):
        yield x


def hash_string(string):
    """The djb2 string hash algorithm from the old comp.lang.c days.  Not a
    terrific hash, but we just need a pseudorandom number based on the string.
    This will do.

    This is used instead of the built-in hash function so that the results
    will be consistent on other systems.  See
    http://stackoverflow.com/questions/793761/built-in-python-hash-function.
    """
    h = 5381

    for c in string:
        h = h * 33 + ord(c)

    return h & 0x0ffffffff


def random_data(type, name, offset):
    """Generate pseudorandom data.

    The data generated is based on the type, name of the field, and offset of
    the member in the UBO.
    """
    if isscalar(type):
        h = hash_string("{}@{}".format(offset, name))

        if type == "int":
            return str(h - 0x7fffffff)
        elif type == "uint":
            return str(h)
        elif type == "bool":
            return str(int((h & 8) == 0))
        elif type == "float" or type == "double":
            return str(float(h - 0x7fffffff) / 65535.0)
        else:
            raise Exception("Unknown scalar type {}".format(type))

    if isvector(type):
        scalar = vector_base_type(type)

        x = [random_data(scalar, name, offset + (i * 3))
             for i in xrange(vector_size(type))]
        return " ".join(x)

    if ismatrix(type):
        r, c = matrix_dimensions(type)

        x = [random_data("float", name, offset + (i * 7))
             for i in xrange(r * c)]
        return " ".join(x)


def generate_test_vectors(fields,
                          field_layouts,
                          block_name,
                          instance_name,
                          packing,
                          row_major):
    test_vectors = []

    for m in iterate_all_block_members(fields,
                                       field_layouts,
                                       block_name,
                                       instance_name,
                                       packing,
                                       row_major):
        a = packing.base_alignment(m.GLSL_type, m.row_major)

        if isarray(m.GLSL_type):
            size = array_elements(m.GLSL_type)
            base_type = array_base_type(m.GLSL_type)
            astride = packing.array_stride(m.GLSL_type, m.row_major)
            name = m.API_name + "[0]"
            while(isarray(base_type)):
                size = array_elements(base_type)
                base_type = array_base_type(base_type)
                name = name + "[0]"
        else:
            base_type = m.GLSL_type
            astride = 0
            name = m.API_name
            size = m.size

        if ismatrix(base_type):
            test_vectors.append((
                    name,
                    m.API_type,
                    size,
                    align(m.offset, a),
                    astride,
                    packing.matrix_stride(base_type, m.row_major),
                    int(m.row_major)))
        elif isvector(base_type) or isscalar(base_type):
            test_vectors.append((
                    name,
                    m.API_type,
                    size,
                    align(m.offset, a),
                    astride,
                    0,
                    0))

    return test_vectors


def scalar_derp(type, name, data):
    """Return a GLSL code string to compare a scalar with its expected value."""
    if type == "bool":
        if int(data) == 0:
            return name
        else:
            return "!" + name
    elif type == "uint":
        return "{} != {}u".format(name, data)
    elif type == "int":
        return "{} != {}".format(name, data)
    elif type == "float":
        # Not all implementations support the bit-cast operators that are used
        # to do bit-exact comparisons.  For this reason float_match needs the
        # float data and the bit-exact data.

        bits = bit_exact_data(data, "float")
        return "!float_match({}, {}, {}u)".format(name, data, bits)
    elif type == "double":
        bits = bit_exact_data(data, "double")

        # 0xHHHHHHHHLLLLLLLL
        # 012345678901234567

        hi = "0x" + bits[2:10]
        lo = "0x" + bits[10:18]

        return "!double_match({}, uvec2({}, {}))".format(name, lo, hi)
    else:
        raise Exception("Unknown scalar type {}".format(type))


def vector_derp(type, name, data):
    """Return a list of GLSL code strings that compare each field of a vector
    its expected value.
    """
    scalar = vector_base_type(type)
    components = ["x", "y", "z", "w"]

    return [scalar_derp(scalar,
                 "{}.{}".format(name, "xyzw"[i]),
                 data[i])
            for i in xrange(vector_size(type))]


def matrix_derp(type, name, data):
    """Return a list of GLSL code strings that compare each field of a matrix
    its expected value.
    """
    c, r = matrix_dimensions(type)

    if type[0] == 'd':
        column_type = "dvec{}".format(r)
    else:
        column_type = "vec{}".format(r)

    data_pairs = []

    for i in xrange(c):
        data_pairs.extend(vector_derp(
                column_type,
                "{}[{}]".format(name, i),
                data[(i * r):(i * r) + r]))

    return data_pairs


def fudge_type_for_setter(type):
    """Return the correct type for the GL API to set the uniform.

    Most GLSL data types are set using a the enum name for the actual type.
    The sole exception is bool and related vector types.  For bool types, the
    same-sized (scalar or vector) integer type is used.
    """
    if type[0] == 'b':
        if type == "bool":
            return "int"
        else:
            return "i" + type[1:]
    else:
        return type


def bit_exact_data(raw_data, type):
    """Several places in the test want bit-exact data for all types.
    For integer types, this is just the raw data.  For floating point (both
    single and double precission) types, the bit-exact data is the hex
    representation of the IEEE 754 encoding.
    """
    if type in ["float", "vec2",   "vec3",   "vec4",
                "mat2",  "mat2x2", "mat2x3", "mat2x4",
                "mat3",  "mat3x2", "mat3x3", "mat3x4",
                "mat4",  "mat4x2", "mat4x3", "mat4x4"]:
        exact_data = []

        for d in raw_data.split(" "):
            p = struct.pack('!f', float(d))
            u = struct.unpack('!I', p)[0]
            exact_data.append(hex(u))

        return " ".join(exact_data)
    elif type in ["double", "dvec2",   "dvec3",   "dvec4",
                  "dmat2",  "dmat2x2", "dmat2x3", "dmat2x4",
                  "dmat3",  "dmat3x2", "dmat3x3", "dmat3x4",
                  "dmat4",  "dmat4x2", "dmat4x3", "dmat4x4"]:
        exact_data = []

        for d in raw_data.split(" "):
            p = struct.pack('!d', float(d))
            u = struct.unpack('!Q', p)[0]

            # Sometimes the hex() generates a spurious "L" at the end of the
            # string.  Only take the first 18 characters to omit the unwanted
            # "L".  I believe this occurs when bit 63 is set.

            exact_data.append(hex(u)[0:18])

        return " ".join(exact_data)
    else:
        return raw_data

def generate_array_data_pairs(name, api_name, element_type, row_major, offset, packing, setters, checkers):
    base_type = array_base_type(element_type)

    astride = packing.array_stride(element_type, row_major)

    for i in xrange(array_elements(element_type)):

        name_with_index = "{}[{}]".format(name, i)
        api_name_with_index = "{}[{}]".format(api_name, i)

        if isarray(base_type):
            offset = offset + (i * arrays_of_arrays_size(base_type) * astride)
            generate_array_data_pairs(name_with_index, api_name_with_index, base_type, row_major, offset, packing, setters, checkers)
        else:
            offset = offset + (i * astride)
            raw_data = random_data(base_type, name_with_index, offset)
            setters.append(
                (fudge_type_for_setter(base_type),
                api_name_with_index,
                bit_exact_data(raw_data, base_type)))

            data = raw_data.split(" ")

            if isscalar(base_type):
                checkers.append(scalar_derp(base_type,
                                            name_with_index,
                                            data[0]))
            elif isvector(base_type):
                checkers.extend(vector_derp(base_type,
                                            name_with_index,
                                            data))
            elif ismatrix(base_type):
                checkers.extend(matrix_derp(base_type,
                                            name_with_index,
                                            data))

def generate_data_pairs(uniform_blocks, packing):
    """Return GLSL code to test values and shader_runner code to set them.

    For each element in each uniform block, two lists are generated, and these
    lists are returned in a tuple of (checkers, setters).  The "checkers" list
    is a list of GLSL comparisons that return true if the contents of the
    uniform block element DOES NOT match the expected value.

    The "setters" list is a list of shader_runner "uniform" statements to set
    each uniform block member to an expected value.
    """
    checkers = []
    setters = []

    for (block_name,
         instance_name,
         global_layout,
         block_layout,
         fields,
         field_layouts) in uniform_blocks:
        for m in iterate_all_block_members(
            fields,
            field_layouts,
            block_name,
            instance_name,
            packing,
            block_row_major_default(global_layout, block_layout)):

            if m.API_type:
                if isarray(m.GLSL_type):
                    generate_array_data_pairs(m.GLSL_name, m.API_name, m.GLSL_type, m.row_major, m.offset, packing, setters, checkers)
                else:
                    raw_data = random_data(m.GLSL_type, m.GLSL_name, m.offset)
                    setters.append((fudge_type_for_setter(m.GLSL_type),
                                    m.API_name,
                                    bit_exact_data(raw_data, m.GLSL_type)))

                    data = raw_data.split(" ")

                    if isscalar(m.GLSL_type):
                        checkers.append(scalar_derp(m.GLSL_type,
                                                    m.GLSL_name,
                                                    data[0]))
                    elif isvector(m.GLSL_type):
                        checkers.extend(vector_derp(m.GLSL_type,
                                                    m.GLSL_name,
                                                    data))
                    elif ismatrix(m.GLSL_type):
                        checkers.extend(matrix_derp(m.GLSL_type,
                                                    m.GLSL_name,
                                                    data))

    return checkers, setters


def pretty_format_type_data(packing, type, offset, row_major):
    """Return a string describing the layout of the type

    This string is intended to be a comment embedded in the GLSL shader code
    that is generated.
    """
    a = packing.base_alignment(type, row_major)
    aligned_offset = align(offset, a)
    size = packing.size(type, row_major)

    row_major_str = "-"
    mstride = "-"
    astride = "-"

    if isarray(type):
        astride = packing.array_stride(type, row_major)

        base_type = without_array(type)
        if ismatrix(base_type) and row_major:
            if row_major:
                row_major_str = "yes"
            else:
                row_major_str = "no"

            mstride = packing.matrix_stride(base_type, row_major)
    else:
        if ismatrix(type):
            if row_major:
                row_major_str = "yes"
            else:
                row_major_str = "no"

            mstride = packing.matrix_stride(type, row_major)

    return "{base_align:>3}  {base_offset:>4}  {aligned_offset:>5}  {padded_size:>6}  {row_major:^5}  {array_stride:>6}  {matrix_stride:>6}".format(
        base_align=a,
        base_offset=offset,
        aligned_offset=aligned_offset,
        padded_size=size,
        row_major=row_major_str,
        array_stride=astride,
        matrix_stride=mstride
        )


def pretty_format_member(m, packing):
    """Return a string of GLSL code to declare the member.

    m -- a block_member object
    packing -- packing rules

    If the member is "nested" inside a structure, the string returned will be
    a comment simply describing the layout of the submember.
    """
    # If the name ends in an array subscript, emit a special line to note that
    # the following fields are the contents of an element of an array of
    # structures.
    if m.GLSL_name[-1] == "]":
        n = m.struct_nesting() + 1
        indent = "//  " + ("  " * n)

        return "{indent}{index}".format(indent=indent,
                                         index=m.GLSL_name[m.GLSL_name.index("["):])

    # Strip off everything before the last period.
    name = m.GLSL_name.split(".")[-1]

    n = m.struct_nesting()
    if n > 0:
        indent = "//  " + ("  " * n)
        field_str = "{indent}{type:<11} {name:<20}".format(
            indent=indent,
            type=m.GLSL_type,
            name=name)[0:31]
    else:
        field_str = "    {type:<11}{name};{padding}//   ".format(
            type=m.GLSL_type,
            name=name,
            padding="          "[len(name):])

    data_str = pretty_format_type_data(
        packing,
        m.GLSL_type,
        m.offset,
        m.row_major)

    # If there is an explicit layout for the member, prepend it to the member
    # declaration.  This also means that the member must be contained directly
    # in the UBO (i.e., not nested in a struct), so no additional indentation
    # is necessary.

    if m.explicit_layout and "#" not in m.explicit_layout:
        return "    layout({layout})\n{field}{data}".format(
            layout=m.explicit_layout,
            field=field_str,
            data=data_str)
    else:
        return "{field}{data}".format(field=field_str, data=data_str)


def block_row_major_default(global_layout, block_layout):
    """Return true if the default matrix layout for the block is row-major."""
    row_major = global_layout is not None and "row_major" in global_layout

    if block_layout:
        if "row_major" in block_layout:
            row_major = True
        elif "column_major" in block_layout:
            # The block layout can override a previous global layout.
            row_major = False

    return row_major


def generate_block_list(glsl_version, packing, ubo_fields, layouts):
    """Return list of uniform blocks to be passed to emit_shader_test.

    The supplied ubo_fields and layouts are reformatted as an element in a
    list of blocks that can be consumed by emit_shader_test.  Depending on the
    value of glsl_version, additional blocks may be added to the list.  The
    new blocks will have the same layout as the original, but the layout will
    be specified in different ways (e.g., by using a global layout qualifier).
    """

    blocks = [("UB1", "", None, packing.layout_string(), ubo_fields, layouts)]

    # If the GLSL version is at least 1.50, UBO functionality is significantly
    # extended.
    #
    # 1. Uniform blocks can have instance names.  The existence of the name
    #    changes the way the block is queried through the API (with the block
    #    name) and the way it is accessed by the shader (with the instance
    #    name).
    #
    # 2. Uniform blocks can be grouped in arrays.  UBO arrays must have an
    #    instance name.
    #
    # This is used to make the tests dramatically more complex.  Each UBO is
    # emitted three times.
    #
    # 1. Without an instance name.
    #
    # 2. With an instance name and the per-block matrix layout switched to
    #    row_major.  The declared layout of the individual fields is modified
    #    so that this block has the same layout as the previous block.
    #
    # 3. With an instance name and an array size.  The per-block matrix layout
    #    is empty, but the global matrix layout is changed to row_major.  This
    #    block should have the same layout as the previous two.

    if glsl_version >= 150:
        inverted_layouts = [relayout_for_default_row_major(l) for l in layouts]

        blocks.append(("UB2",
                       "ub2",
                       None,
                       packing.layout_string() + ", row_major",
                       ubo_fields,
                       inverted_layouts))

        blocks.append(("UB3",
                       "ub3",
        # Disabled to work around Mesa bug #83508.
        #               "ub3[2]",
                       packing.layout_string() + ", row_major",
                       None,
                       ubo_fields,
                       inverted_layouts))

    return blocks


def emit_shader_test(blocks, packing, glsl_version, extensions):
    structures = []
    test_vectors = []

    for (block_name,
         instance_name,
         global_layout,
         block_layout,
         fields,
         field_layouts) in blocks:

        structures.extend([s for s in iterate_structures(fields)])

        test_vectors.extend(generate_test_vectors(
                fields,
                field_layouts,
                block_name,
                instance_name,
                packing,
                block_row_major_default(global_layout, block_layout)))


    checkers, setters = generate_data_pairs(blocks, packing)

    # If the GLSL version is at least 1.40, UBOs are already supported, and we
    # don't need to enable the extension.

    if glsl_version >= 140 and "GL_ARB_uniform_buffer_object" in extensions:
        extensions.remove("GL_ARB_uniform_buffer_object")

    t = Template(dedent("""\
    [require]
    GLSL >= ${glsl_version / 100}.${glsl_version % 100}
    % for ext in extensions:
    ${ext}
    % endfor

    # Do NOT edit the following lines.
    # GLSL ${glsl_version}
    # EXTENSIONS ${extensions}
    # PACKING ${packing.layout_string()}
    % for s in structures:
    # STRUCT ("${s}", ${struct_types[s]})
    % endfor
    % for b in uniform_blocks:
    # UBO ${b}
    % endfor
    # DATA END

    [vertex shader]
    % for ext in extensions:
    #extension ${ext}: require
    % endfor
    #extension GL_ARB_shader_bit_encoding: enable
    #extension GL_ARB_gpu_shader5: enable

    precision highp float;
    % for s in structures:

    struct ${s} {
        % for (field_type, field_name) in struct_types[s]:
        ${"{:<11}".format(field_type)} ${field_name};
        % endfor
    };
    % endfor

    % for (block_name, instance_name, global_layout, block_layout, fields, field_layouts) in uniform_blocks:
    % if global_layout:
    layout(${global_layout}) uniform;

    % endif
    % if block_layout:
    layout(${block_layout})
    % endif
    uniform ${block_name} {
                              // base   base  align  padded  row-   array   matrix
                              // align  off.  off.   size    major  stride  stride
    % for m in iterate_all_block_members(fields, field_layouts, block_name, instance_name, packing, block_row_major_default(global_layout, block_layout)):
    ${pretty_format_member(m, packing)}
    % endfor
    } ${instance_name};
    % endfor

    flat out int vertex_pass;
    in vec4 piglit_vertex;

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
        /* std140 (or shared) layout prevents any fields or blocks from being
         * eliminated.  Section 2.11.6 of the OpenGL ES 3.0 spec makes this
         * explicit, but desktop GL specs only imply it.
         */
        bool pass = true;

    % for i in xrange(len(checkers)):
        % if i % 5 == 0:
        if (${checkers[i]})
            pass = false;
        % endif
    % endfor

        vertex_pass = int(pass);
        gl_Position = piglit_vertex;
    }

    [fragment shader]
    precision highp float;

    out vec4 piglit_fragcolor;
    flat in int vertex_pass;

    void main()
    {
        piglit_fragcolor = bool(vertex_pass) ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);
    }

    [test]
    link success
    % for (name, type, size, offset, astride, mstride, row_major) in test_vectors:

    active uniform ${name} GL_UNIFORM_TYPE ${type}
    active uniform ${name} GL_UNIFORM_SIZE ${size}
        % if packing.fixed_offsets():
    active uniform ${name} GL_UNIFORM_OFFSET ${offset}
    active uniform ${name} GL_UNIFORM_ARRAY_STRIDE ${astride}
    active uniform ${name} GL_UNIFORM_MATRIX_STRIDE ${mstride}
        % endif
    active uniform ${name} GL_UNIFORM_IS_ROW_MAJOR ${row_major}
    % endfor

    % for (type, name, data) in setters:
    uniform ${type} ${name} ${data}
    % endfor

    draw rect -1 -1 2 2
    probe all rgba 0.0 1.0 0.0 1.0"""))

    return t.render(glsl_version=glsl_version,
                    extensions=extensions,
                    structures=structures,
                    test_vectors=test_vectors,
                    uniform_blocks=blocks,
                    packing=packing,
                    iterate_all_block_members=iterate_all_block_members,
                    pretty_format_member=pretty_format_member,
                    block_row_major_default=block_row_major_default,
                    struct_types=struct_types,
                    checkers=checkers,
                    setters=setters)


def generate_file_name(requirements, packing):
    """Based on the required UBO elements and the packing, generate a
    plausible file name for the test case.
    """
    prefix = packing.layout_string() + "-"
    suffix = ".shader_test"

    body = "-and-".join(["-".join(req) for req in requirements])

    return prefix + body + suffix


class packing_rules(object):
    __metaclass__ = abc.ABCMeta

    @abc.abstractproperty
    def layout_string(self):
        """Get the string used in a layout qualifier to select this set of
        layout rules.
        """

    @abc.abstractproperty
    def fixed_offsets(self):
        """Return true if fields in this layout have fixed locations (e.g.,
        std140) or false if they can vary among implementations (e.g., shared
        or packed)?
        """

    @abc.abstractmethod
    def base_alignment(self, type, row_major):
        """Determine the base alignment, in bytes, of the named type"""

    @abc.abstractmethod
    def matrix_stride(self, type, row_major):
        """Determine the stride, in bytes, from one indexable vector of the
        matrix (column or row depending on the orientation) to the next.
        """

    @abc.abstractmethod
    def array_stride(self, type, row_major):
        """Determine the stride, in bytes, from one array element to the next.
        If the type is not an array type, zero is returned.
        """

    def size(self, type, row_major):
        """Determine the size, in bytes, of the specified type.

        Applying the packing rules and the row-major setting, determine the
        size, in bytes, of the entire data type.  This will include padded
        after the data element as required by the GLSL specification.
        """
        if isarray(type):
            return self.array_stride(type, row_major) * arrays_of_arrays_size(type)

        if type in ["float", "bool", "int", "uint"]:
            return 4

        if type == "double":
            return 8

        if type in ["vec2", "bvec2", "ivec2", "uvec2"]:
            return 2 * 4

        if type == "dvec2":
            return 2 * 8

        if type in ["vec3", "bvec3", "ivec3", "uvec3"]:
            return 3 * 4

        if type == "dvec3":
            return 3 * 8

        if type in ["vec4", "bvec4", "ivec4", "uvec4"]:
            return 4 * 4

        if type == "dvec4":
            return 4 * 8

        if "mat" in type:
            c, r = matrix_dimensions(type)
            if not row_major:
                return c * self.matrix_stride(type, row_major)
            else:
                return r * self.matrix_stride(type, row_major)

        if type not in struct_types:
            raise Exception("Unknown type {}".format(type))

        s = 0
        fields = struct_types[type]
        for (t, n) in fields:
            a = self.base_alignment(t, row_major)

            s = align(s, a) + self.size(t, row_major)

        s = align(s, self.base_alignment(type, row_major))
        return s


class std140_packing_rules(packing_rules):
    def layout_string(self):
        return "std140"

    def fixed_offsets(self):
        return True

    def base_alignment(self, type, row_major):
        # (4) If the member is an array of scalars or vectors, the base
        #     alignment and array stride are set to match the base alignment
        #     of a single array element, according to rules (1), (2), and (3),
        #     and rounded up to the base alignment of a vec4. The array may
        #     have padding at the end; the base offset of the member following
        #     the array is rounded up to the next multiple of the base
        #     alignment.

        if isarray(type):
            return max(16,
                       self.base_alignment(array_base_type(type), row_major))

        # (1) If the member is a scalar consuming <N> basic machine units, the
        #     base alignment is <N>.

        if isscalar(type):
            return basic_machine_units(type)

        if isvector(type):
            # (2) If the member is a two- or four-component vector with
            #     components consuming <N> basic machine units, the base
            #     alignment is 2<N> or 4<N>, respectively.
            #
            # (3) If the member is a three-component vector with components
            #     consuming <N> basic machine units, the base alignment is
            #     4<N>.

            components = vector_size(type)
            if components == 2 or components == 4:
                return components * basic_machine_units(vector_base_type(type))
            elif components == 3:
                return 4 * basic_machine_units(vector_base_type(type))

            raise Exception("Invalid vector size {} for type {}".format(
                components,
                type))
        elif ismatrix(type):
            return self.matrix_stride(type, row_major)

        if type not in struct_types:
            raise Exception("Unknown type {}".format(type))

        # (9) If the member is a structure, the base alignment of the
        #     structure is <N>, where <N> is the largest base alignment value
        #     of any of its members, and rounded up to the base alignment of a
        #     vec4. The individual members of this sub-structure are then
        #     assigned offsets by applying this set of rules recursively,
        #     where the base offset of the first member of the sub-structure
        #     is equal to the aligned offset of the structure. The structure
        #     may have padding at the end; the base offset of the member
        #     following the sub-structure is rounded up to the next multiple
        #     of the base alignment of the structure.

        a = 16
        fields = struct_types[type]
        for (field_type, field_name) in fields:
            a = max(a, self.base_alignment(field_type, row_major))

        return a

    def matrix_stride(self, type, row_major):
        c, r = matrix_dimensions(type)
        if not row_major:
            # (4) If the member is an array of scalars or vectors, the base
            #     alignment and array stride are set to match the base
            #     alignment of a single array element, according to rules (1),
            #     (2), and (3), and rounded up to the base alignment of a
            #     vec4. The array may have padding at the end; the base offset
            #     of the member following the array is rounded up to the next
            #     multiple of the base alignment.
            #
            # (5) If the member is a column-major matrix with <C> columns and
            #     <R> rows, the matrix is stored identically to an array of
            #     <C> column vectors with <R> components each, according to
            #     rule (4).

            if type[0] == 'd':
                return max(16, self.base_alignment("dvec{}".format(r), False))
            else:
                return max(16, self.base_alignment("vec{}".format(r), False))
        else:
            # (7) If the member is a row-major matrix with <C> columns and <R>
            #     rows, the matrix is stored identically to an array of <R>
            #     row vectors with <C> components each, according to rule (4).

            if type[0] == 'd':
                return max(16, self.base_alignment("dvec{}".format(c), False))
            else:
                return max(16, self.base_alignment("vec{}".format(c), False))

    def array_stride(self, type, row_major):
        base_type = without_array(type)

        if not isstructure(base_type):
            # (4) If the member is an array of scalars or vectors, the base
            #     alignment and array stride are set to match the base
            #     alignment of a single array element, according to rules (1),
            #     (2), and (3), and rounded up to the base alignment of a
            #     vec4. The array may have padding at the end; the base offset
            #     of the member following the array is rounded up to the next
            #     multiple of the base alignment.
            return max(16,
                       max(self.base_alignment(base_type, row_major),
                           self.size(base_type, row_major)))
        else:
            # (9) If the member is a structure, the base alignment of the
            #     structure is <N>, where <N> is the largest base alignment
            #     value of any of its members, and rounded up to the base
            #     alignment of a vec4. The individual members of this
            #     sub-structure are then assigned offsets by applying this set
            #     of rules recursively, where the base offset of the first
            #     member of the sub-structure is equal to the aligned offset
            #     of the structure. The structure may have padding at the end;
            #     the base offset of the member following the sub-structure is
            #     rounded up to the next multiple of the base alignment of the
            #     structure.
            #
            # (10) If the member is an array of <S> structures, the <S> elements
            #     of the array are laid out in order, according to rule (9).

            return align(self.size(base_type, row_major),
                         self.base_alignment(base_type, row_major))


class shared_packing_rules(std140_packing_rules):
    def layout_string(self):
        return "shared"

    def fixed_offsets(self):
        return False


class unique_name_dict:
    """Helper class to generate a unique name for each field.

    Fields for each particular type have names based on the type.  Each name
    is suffixed by a number, and the number is incremented each time a field
    of that type is created.  For example, the first int field will be named
    "i1", and the second will be named "i2".
    """

    def __init__(self):
        self.names = {}

    def trim_name(self, type):
        """Internal method to reduce a type name to a canonical form

        These names are used to track the type in the self.names dict.

        Array types have the arrayness stripped.  Vector types have vector
        count stripped.  Matrix types are converted to canonical "mat#x#"
        form.  All other type names are unmodified.
        """
        if isarray(type):
            t = without_array(type)
        else:
            t = type

        if ismatrix(t):
            # Canonicalize matrix type names.
            c, r = matrix_dimensions(t)

            name = "mat{}x{}".format(c, r)
            if t[0] == "d":
                name = "d" + name

            return name
        elif isscalar(t):
            return t
        elif isvector:
            return t.strip("1234")
        else:
            # Assume it must be a structure.
            return t

    def add_type(self, type):
        """Internal method to add a previously unknown type to the dictionary

        type -- Actual GLSL type name

        Generates the base name for fields of this type.  Adds a tuple to dict
        (indexed by "type") of (base_name, 1).
        """
        if isarray(type):
            t = without_array(type)
        else:
            t = type

        if isvector(t):
            base = "{}v".format(vector_base_type(t)[0])
        elif ismatrix(t):
            c, r = matrix_dimensions(t)

            if t[0] == 'd':
                base = "dm{}{}_".format(c, r)
            else:
                base = "m{}{}_".format(c, r)
        elif isscalar(t):
            base = t[0]
        elif t[0] == "S":
            base = "s{}_".format(t[1:])
        else:
            raise Exception("Malformed type name {}".format(t))

        self.names[self.trim_name(t)] = (base, 1)
        return

    def get_name(self, type):
        """Return a unique name for a field of the specified type."""
        t = self.trim_name(type)
        if t not in self.names:
            self.add_type(type)

        base, count = self.names[t]
        self.names[t] = (base, count + 1)

        return "{}{}".format(base, count)


class block_member(object):
    """Helper class to track all of the layout information for a single
    (sub-)member of a uniform block."""

    TYPE_ENUM = {
        'float': "GL_FLOAT",
        'vec2':  "GL_FLOAT_VEC2",
        'vec3':  "GL_FLOAT_VEC3",
        'vec4':  "GL_FLOAT_VEC4",

        'double': "GL_DOUBLE",
        'dvec2':  "GL_DOUBLE_VEC2",
        'dvec3':  "GL_DOUBLE_VEC3",
        'dvec4':  "GL_DOUBLE_VEC4",

        'int':   "GL_INT",
        'ivec2': "GL_INT_VEC2",
        'ivec3': "GL_INT_VEC3",
        'ivec4': "GL_INT_VEC4",

        'uint':  "GL_UNSIGNED_INT",
        'uvec2': "GL_UNSIGNED_INT_VEC2",
        'uvec3': "GL_UNSIGNED_INT_VEC3",
        'uvec4': "GL_UNSIGNED_INT_VEC4",

        'bool':  "GL_BOOL",
        'bvec2': "GL_BOOL_VEC2",
        'bvec3': "GL_BOOL_VEC3",
        'bvec4': "GL_BOOL_VEC4",

        'mat2':   "GL_FLOAT_MAT2",
        'mat2x2': "GL_FLOAT_MAT2",
        'mat2x3': "GL_FLOAT_MAT2x3",
        'mat2x4': "GL_FLOAT_MAT2x4",

        'mat3':   "GL_FLOAT_MAT3",
        'mat3x2': "GL_FLOAT_MAT3x2",
        'mat3x3': "GL_FLOAT_MAT3",
        'mat3x4': "GL_FLOAT_MAT3x4",

        'mat4':   "GL_FLOAT_MAT4",
        'mat4x2': "GL_FLOAT_MAT4x2",
        'mat4x3': "GL_FLOAT_MAT4x3",
        'mat4x4': "GL_FLOAT_MAT4",

        'dmat2':   "GL_DOUBLE_MAT2",
        'dmat2x2': "GL_DOUBLE_MAT2",
        'dmat2x3': "GL_DOUBLE_MAT2x3",
        'dmat2x4': "GL_DOUBLE_MAT2x4",

        'dmat3':   "GL_DOUBLE_MAT3",
        'dmat3x2': "GL_DOUBLE_MAT3x2",
        'dmat3x3': "GL_DOUBLE_MAT3",
        'dmat3x4': "GL_DOUBLE_MAT3x4",

        'dmat4':   "GL_DOUBLE_MAT4",
        'dmat4x2': "GL_DOUBLE_MAT4x2",
        'dmat4x3': "GL_DOUBLE_MAT4x3",
        'dmat4x4': "GL_DOUBLE_MAT4"
    }

    def __init__(self,
                 GLSL_name,
                 API_name,
                 GLSL_type,
                 explicit_layout,
                 offset,
                 row_major):
        self.GLSL_name = GLSL_name
        self.GLSL_type = GLSL_type

        self.API_name = API_name

        self.explicit_layout = explicit_layout
        self.offset = offset
        self.row_major = row_major

        if isarray(GLSL_type):
            base_type = without_array(GLSL_type)
            if isstructure(base_type):
                self.API_type = None
            else:
                self.API_type = self.TYPE_ENUM[base_type];

            self.size = array_elements(GLSL_type)
        elif isstructure(GLSL_type):
            self.API_type = None
            self.size = 1
        else:
            self.API_type = self.TYPE_ENUM[GLSL_type];
            self.size = 1

    def struct_nesting(self):
        if "." in self.GLSL_name:
            # If the block has an instance name, the API name will use the
            # block name instead of the instance name.  As a result,
            # GLSL_name and API_name will be different.
            #
            # The first "." is for the block instance name, so it does not count
            # as structure nesting.

            if self.GLSL_name != self.API_name:
                return collections.Counter(self.GLSL_name)["."] - 1
            else:
                return collections.Counter(self.GLSL_name)["."]
        else:
            return 0


if __name__ == "__main__":
    if len(sys.argv) > 1:
        max_glsl_version = int(sys.argv[1])
    else:
        max_glsl_version = 130

    if len(sys.argv) > 2:
        extensions = sys.argv[2:]
    else:
        extensions = []

    # Pick a random GLSL version from the available set of possible versions.
    glsl_version = random.choice([v for v in [130, 140, 150, 400, 430]
                                  if v <= max_glsl_version])

    # Use the GLSL version filter out some extensions that are redundant.
    if glsl_version >= 140 and "GL_ARB_uniform_buffer_object" in extensions:
        extensions.remove("GL_ARB_uniform_buffer_object")

    if glsl_version >= 400 and "GL_ARB_gpu_shader_fp64" in extensions:
        extensions.remove("GL_ARB_gpu_shader_fp64")

    if glsl_version >= 430 and "GL_ARB_arrays_of_arrays" in extensions:
        extensions.remove("GL_ARB_arrays_of_arrays")

    # Pick a random subset of the remaining extensions.
    num_ext = len(extensions)
    if num_ext > 0:
        random.shuffle(extensions)
        r = random.randint(0, num_ext)
        extensions = extensions[:r]

    # Based on the GLSL version and the set of extensions, pick the set of
    # possible data types.
    if glsl_version < 400:
        types = ALL130_TYPES
    else:
        types = ALL400_TYPES

    if "GL_ARB_gpu_shader_fp64" in extensions:
        types.extend(DOUBLE_TYPES)

    # Based on the GLSL version, pick a set of packing rules
    # FINISHME: Add support for std430_packing_rules() soon.
    packing = random.choice([std140_packing_rules(), shared_packing_rules()])

    # Based on the GLSL version and the set of available extensions, pick
    # some required combinations of data structures to include in the UBO.
    arrays_of_arrays = (glsl_version >= 430 or
                        "GL_ARB_arrays_of_arrays" in extensions)

    allow_row_major_structure = glsl_version >= 150

    requirements = []
    for i in [1, 2]:
        x = [random.choice(["array", "struct"])]

        for j in [1, 2, 3]:
            # If arrays-of-arrays are not supported, don't allow "array" to be
            # picked twice in a row.

            if x[-1] == "array" and not arrays_of_arrays:
                x.append("struct")
            else:
                x.append(random.choice(["array", "struct"]))

        if "struct" in x and allow_row_major_structure:
            ordering = random.choice([None,
                                      None,
                                      None,
                                      None,
                                      "column_major",
                                      "#column_major",
                                      "row_major",
                                      "row_major"])
            if ordering:
                x = [ordering] + x

        requirements.append(x)

    if glsl_version < 140:
        extensions.append("GL_ARB_uniform_buffer_object")

    # Generate the test!
    fields, required_layouts = generate_ubo(requirements, types)

    layouts = generate_layouts(
        fields,
        required_layouts,
        allow_row_major_structure)

    blocks = generate_block_list(
        glsl_version,
        packing,
        fields,
        layouts)

    print(emit_shader_test(
        blocks,
        packing,
        glsl_version,
        extensions))
