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

import os
import errno
import random_ubo

def do_test(requirements, packing):
    path = os.path.join("spec", "arb_uniform_buffer_object", "execution")

    try:
        os.makedirs(path)
    except OSError as exc:
        if exc.errno == errno.EEXIST and os.path.isdir(path):
            pass
        else:
            raise
        pass

    basename = random_ubo.generate_file_name(requirements, packing)
    fullname = os.path.join(path, basename)

    file = open(fullname, "w", 0)

    fields, required_layouts = random_ubo.generate_ubo(
        requirements,
        random_ubo.ALL130_TYPES)

    layouts = random_ubo.generate_layouts(
        fields,
        required_layouts,
        # Due to bugs in the NVIDIA closed-source driver, do not randomly
        # generate layout(row_major) on structures.  Several tests will,
        # however, do this explicitly.
        False)

    blocks = random_ubo.generate_block_list(
        130,
        packing,
        fields,
        layouts)

    print basename
    file.write(random_ubo.emit_shader_test(
        blocks,
        packing,
        130,
        ["GL_ARB_uniform_buffer_object","GL_ARB_arrays_of_arrays"]))

    file.close()

all_packing = [random_ubo.std140_packing_rules(),
               random_ubo.shared_packing_rules()]

all_requirements = []

# Generate a test for each matrix type that:
#
# - Explicitly declares one as row-major and another as column-major.
#
# - Embeds the matrix in a structure without a layout.
#
# - Embeds the matrix in a structure with a row-major layout.
#
# - Embeds the matrix in a structure with a column-major layout.
#
# - Each of the above in an array

for m in ["mat2x2", "mat2x3", "mat2x4",
          "mat3x2", "mat3x3", "mat3x4",
          "mat4x2", "mat4x3", "mat4x4"]:
    all_requirements.append([["row_major", m], ["column_major", m]])
    all_requirements.append([["#column_major", "struct", m]])
    all_requirements.append([["row_major",     "struct", m]])
    all_requirements.append([["column_major",  "struct", m]])

    all_requirements.append([["row_major", "array", m],
                             ["column_major", "array", m]])
    all_requirements.append([["#column_major", "struct", "array", m]])
    all_requirements.append([["row_major",     "struct", "array", m]])
    all_requirements.append([["column_major",  "struct", "array", m]])

    all_requirements.append([["#column_major", "array", "struct", m]])
    all_requirements.append([["row_major",     "array", "struct", m]])
    all_requirements.append([["column_major",  "array", "struct", m]])

    all_requirements.append([["#column_major", "array", "struct", "array", m]])
    all_requirements.append([["row_major",     "array", "struct", "array", m]])
    all_requirements.append([["column_major",  "array", "struct", "array", m]])

# Also add some struct-nesting tests.

all_requirements.append([["struct", "struct"]])
all_requirements.append([["struct", "struct", "struct"]])
all_requirements.append([["struct", "array", "struct"]])
all_requirements.append([["array", "struct", "struct"]])
all_requirements.append([["array", "struct", "array", "struct"]])

all_requirements.append([["struct", "array", "array", "struct"]])
all_requirements.append([["struct", "array", "array", "array", "struct"]])
all_requirements.append([["array", "array", "struct", "array"]])
all_requirements.append([["struct", "array", "array", "array"]])

for p in all_packing:
    for r in all_requirements:
        do_test(r, p)
