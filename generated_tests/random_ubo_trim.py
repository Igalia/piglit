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
import os
import sys
import errno
import ast
import random
import random_ubo

from random_ubo import struct_types

def remove_empty_structure(s, do_remove = True):
    global struct_types

    removed = [s]

    for x in struct_types:
        # Delete the empty structure at the end, and the structure
        # cannot contain itself.
        if s == x:
            continue

        # A previous caller may be in the process of deleting this structure
        # type, so just skip it for now.
        if len(struct_types[x]) == 0:
            continue

        i = 0
        while i < len(struct_types[x]):
            field_type, field_name = struct_types[x][i]

            if random_ubo.isarray(field_type):
                field_type = random_ubo.array_base_type(field_type)

            if field_type == s:
                del struct_types[x][i]
            else:
                i = i + 1

        # Now x could be empty, so possibly remove it too.
        if len(struct_types[x]) == 0:
            removed.extend(remove_empty_structure(x, False))


    if do_remove:
        for x in removed:
            del struct_types[x]

    return removed


def diminish_array_type(fields, i):
    field_type, field_name = fields[i]

    if not random_ubo.isarray(field_type):
        return False

    if random_ubo.array_elements(field_type) == 1:
        smaller_type = random_ubo.array_base_type(field_type)
    else:
        smaller_type = random_ubo.array_base_type(field_type) + "[1]"

    print("{} => {}".format(field_type, smaller_type))
    fields[i] = (smaller_type, field_name)
    return True


def remove_random_field(blocks):
    global struct_types

    potential_kill_list = []

    for b in blocks:
        potential_kill_list.extend([(b[0], i)
                                    for i in xrange(len(b[4]))])

    for s in struct_types:
        potential_kill_list.extend([(s, i)
                                    for i in xrange(len(struct_types[s]))])

    if len(potential_kill_list) == 0:
        return False

    owner, i = random.choice(potential_kill_list)

    print("{} field index {}:".format(owner, i), end="")

    if owner in struct_types:
        if diminish_array_type(struct_types[owner], i):
            return True

        print("remove {}".format(struct_types[owner][i]))
        del struct_types[owner][i]

        if len(struct_types[owner]) == 0:
            removed = remove_empty_structure(owner)

            # Update the UBOs to note that some structures are gone.

            if len(removed) != 0:
                for (block_name,
                     instance_name,
                     global_layout,
                     block_layout,
                     fields,
                     field_layouts) in blocks:
                    j = 0
                    while j < len(fields):
                        field_type, field_name = fields[j]

                        if random_ubo.isarray(field_type):
                            field_type = random_ubo.array_base_type(field_type)

                        if field_type in removed:
                            del fields[j]
                            del field_layouts[j]
                        else:
                            j = j + 1

    else:
        for b in blocks:
            if b[0] == owner:
                if not diminish_array_type(b[4], i):
                    print("remove {}".format(b[4][i]))

                    # Delete the field
                    del b[4][i]
                    # Delete the layout
                    del b[5][i]

    # Remove any potentially empty UBOs
    i = 0
    while i < len(blocks):
        if len(blocks[i][4]) == 0:
            del blocks[i]
        else:
            i = i + 1

    return True

if len(sys.argv) <= 2:
    print("Usage: {} input output".format(sys.argv[0]))
    sys.exit(1)

file_in = open(sys.argv[1], "r", 0)
file_out = open(sys.argv[2], "w", 0)

glsl_version = None
extensions = None
packing = None
blocks = []

for line in file_in:
    if line[0] != '#':
        continue

    if line.startswith("# GLSL"):
        glsl_version = int(line.split(" ")[2])
    elif line.startswith("# EXTENSIONS"):
        extensions = ast.literal_eval(line[12:].strip())
    elif line.startswith("# PACKING"):
        packing_str = line.split(" ")[2].strip()

        if packing_str == "shared":
            packing = random_ubo.shared_packing_rules()
        elif packing_str == "std140":
            packing = random_ubo.std140_packing_rules()
        else:
            print("Invalid packing string '{}'.".format(packing_str))
            sys.exit(1)
    elif line.startswith("# STRUCT"):
        struct_name, struct_fields = ast.literal_eval(line[8:].strip())
        struct_types[struct_name] = struct_fields
    elif line.startswith("# UBO"):
        blocks.append(ast.literal_eval(line[5:].strip()))
    elif line.startswith("# DATA END"):
        break
    else:
        pass

file_in.close()

if not remove_random_field(blocks):
    sys.exit(1)

if len(blocks) == 0:
    sys.exit(1)

file_out.write(random_ubo.emit_shader_test(
        blocks,
        packing,
        glsl_version,
        extensions))
file_out.write("\n")
file_out.close()
