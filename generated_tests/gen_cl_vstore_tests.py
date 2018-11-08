# coding=utf-8
# Copyright 2016 Advanced Micro Devices, Inc.
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
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

from __future__ import print_function, division, absolute_import
import os
import random
import textwrap

from six.moves import range

from modules import utils

TYPES = ['char', 'uchar', 'short', 'ushort', 'int', 'uint', 'long', 'ulong', 'half', 'float', 'double']
VEC_SIZES = ['2', '3', '4', '8', '16']

DIR_NAME = os.path.join("cl", "vstore")


def gen_array(size):
    random.seed(size)
    return ' '.join([str(random.randint(0, 255)) for i in range(size)])


def ext_req(type_name):
    if type_name[:6] == "double":
        return "require_device_extensions: cl_khr_fp64"
    if type_name[:4] == "half":
        return "require_device_extensions: cl_khr_fp16"
    return ""


def begin_test(suffix, type_name, mem_type, vec_sizes, addr_space, aligned):
    file_name = os.path.join(DIR_NAME, "vstore{}-{}-{}.cl".format(suffix, type_name, addr_space))
    print(file_name)
    f = open(file_name, 'w')
    f.write(textwrap.dedent(("""\
    /*!
    [config]
    name: Vector store{suffix} {addr_space} {type_name}2,3,4,8,16
    clc_version_min: 11

    dimensions: 1
    global_size: 1 0 0
    """ + ext_req(type_name))
    .format(type_name=type_name, addr_space=addr_space, suffix=suffix)))
    for s in vec_sizes:
        size = int(s) if s != '' else 1
        modsize = 4 if size == 3 and aligned else size
        offset = modsize if aligned else 1
        canary= '0xdeadp1' if type_name in ('float', 'double') else '0xdead'

        ty_name = type_name + s
        f.write(textwrap.dedent("""
        [test]
        name: vector store{suffix} {addr_space} {type_name}
        kernel_name: vstore{suffix}{n}_{addr_space}
        arg_out: 0 buffer {mem_type}[{size}] {offset_zeros}{gen_array} {canary}
        arg_in: 0 buffer {mem_type}[{size}] {offset_size_zeros} {canary}
        arg_in:  1 buffer {type_name}[1] {gen_array}

        [test]
        name: vector store{suffix} {addr_space} offset {type_name}
        kernel_name: vstore{suffix}{n}_{addr_space}_offset
        arg_out: 0 buffer {mem_type}[{offset_size}] {offset_zeros} {gen_array} {padd_zeros} {gen_array} {canary}
        arg_in: 0 buffer {mem_type}[{offset_size}] {offset_modsize_size_zeros} {canary}
        arg_in: 1 buffer {type_name}[1] {gen_array}
        """.format(type_name=ty_name, mem_type=mem_type, size=size + offset + 1,
                   offset_zeros = ("0 " * offset),
                   offset_size_zeros = ("0 " * (offset + size)),
                   padd_zeros = ("0 " * (modsize - size)),
                   offset_modsize_size_zeros = ("0 " * (modsize + size + offset)),
                   offset_size=modsize + size + offset + 1, n=s,
                   gen_array=gen_array(size),
                   suffix=suffix, addr_space=addr_space,
                   canary=canary)))

    f.write(textwrap.dedent("""
    !*/
    """))
    if type_name == "double":
        f.write(textwrap.dedent("""
        #pragma OPENCL EXTENSION cl_khr_fp64: enable
        """))
    if type_name == "half":
        f.write(textwrap.dedent("""
        #pragma OPENCL EXTENSION cl_khr_fp16: enable
        """))
    return f


def gen_test_global(suffix, t, mem_type, vec_sizes, aligned):
    f = begin_test(suffix, t, mem_type, vec_sizes, 'global', aligned)
    for s in vec_sizes:
        offset = int(s) if aligned else 1
        offset = 4 if offset == 3 else offset

        type_name = t + s
        f.write(textwrap.dedent("""
        kernel void vstore{suffix}{n}_global(global {mem_type} *out,
                                     global {type_name} *in) {{
            {type_name} tmp = in[0];
            vstore{suffix}{n}(tmp, 0, out + {offset});
        }}

        kernel void vstore{suffix}{n}_global_offset(global {mem_type} *out,
                                            global {type_name} *in) {{
            {type_name} tmp = in[0];
            vstore{suffix}{n}(tmp, 0, out + {offset});
            vstore{suffix}{n}(tmp, 1, out + {offset});
        }}
        """.format(type_name=type_name, mem_type=mem_type, n=s, suffix=suffix,
                   offset=offset)))

    f.close()


def gen_test_local_private(suffix, t, mem_type, vec_sizes, addr_space, aligned):
    f = begin_test(suffix, t, mem_type, vec_sizes, addr_space, aligned)
    for s in vec_sizes:
        size = int(s) if s != '' else 1
        modsize = 4 if size == 3 and aligned else size
        offset = modsize if aligned else 1

        type_name = t + s
        f.write(textwrap.dedent("""
        kernel void vstore{suffix}{n}_{addr_space}(global {mem_type} *out,
                                     global {type_name} *in) {{
            {type_name} tmp = in[0];
            volatile {addr_space} {mem_type} loc[{size}];
            for (int i = 0; i < {size}; ++i)
                loc[i] = ({mem_type})0;

            vstore{suffix}{n}(tmp, 0, ({addr_space} {mem_type}*)loc + {offset});
            for (int i = 0; i < {size}; ++i)
                out[i] = loc[i];
        }}

        kernel void vstore{suffix}{n}_{addr_space}_offset(global {mem_type} *out,
                                            global {type_name} *in) {{
            {type_name} tmp = in[0];
            volatile {addr_space} {mem_type} loc[{offset_size}];
            for (int i = 0; i < {offset_size}; ++i)
                loc[i] = ({mem_type})0;

            vstore{suffix}{n}(tmp, 0, ({addr_space} {mem_type}*)loc + {offset});
            vstore{suffix}{n}(tmp, 1, ({addr_space} {mem_type}*)loc + {offset});
            for (int i = 0; i < {offset_size}; ++i)
                out[i] = loc[i];
        }}
        """.format(type_name=type_name, mem_type=mem_type, n=s, suffix=suffix,
                   offset_size=size + modsize + offset, size=size + offset,
                   addr_space=addr_space, offset=offset)))

    f.close()


# vstore_half is special, because CLC won't allow us to use half type without
# cl_khr_fp16
def gen_test_local_private_half(suffix, t, vec_sizes, addr_space, aligned):
    f = begin_test(suffix, t, 'half', vec_sizes, addr_space, aligned)
    for s in vec_sizes:
        size = int(s) if s != '' else 1
        modsize = 4 if size == 3 and aligned else size
        offset = modsize if aligned else 1

        type_name = t + s
        f.write(textwrap.dedent("""
        kernel void vstore{suffix}{n}_{addr_space}(global half *out,
                                     global {type_name} *in) {{
            {type_name} tmp = in[0];
            volatile {addr_space} short loc[{size}];
            for (int i = 0; i < {size}; ++i)
                loc[i] = 0;

            vstore{suffix}{n}(tmp, 0, ({addr_space} half*)loc + {offset});

            for (int i = 0; i < {size}; ++i)
                ((global short *)out)[i] = loc[i];
        }}

        kernel void vstore{suffix}{n}_{addr_space}_offset(global half *out,
                                            global {type_name} *in) {{
            {type_name} tmp = in[0];
            volatile {addr_space} short loc[{offset_size}];
            for (int i = 0; i < {offset_size}; ++i)
                loc[i] = 0;

            vstore{suffix}{n}(tmp, 0, ({addr_space} half*)loc + {offset});
            vstore{suffix}{n}(tmp, 1, ({addr_space} half*)loc + {offset});

            for (int i = 0; i < {offset_size}; ++i)
                ((global short *)out)[i] = loc[i];
        }}
        """.format(type_name=type_name, n=s, suffix=suffix,
                   offset_size=size + modsize + offset, size=size + offset,
                   addr_space=addr_space, offset=offset)))


def gen_test_local(suffix, t, mem_type, vec_sizes, aligned):
    if mem_type == 'half':
        gen_test_local_private_half(suffix, t, vec_sizes, 'local', aligned)
    else:
        gen_test_local_private(suffix, t, mem_type, vec_sizes, 'local', aligned)


def gen_test_private(suffix, t, mem_type, vec_sizes, aligned):
    if mem_type == 'half':
        gen_test_local_private_half(suffix, t, vec_sizes, 'private', aligned)
    else:
        gen_test_local_private(suffix, t, mem_type, vec_sizes, 'private', aligned)


def main():
    utils.safe_makedirs(DIR_NAME)
    for t in TYPES:
        gen_test_global('', t, t, VEC_SIZES, False);
        gen_test_local('', t, t, VEC_SIZES, False);
        gen_test_private('', t, t, VEC_SIZES, False);

    for aligned in False, True:
        suffix = "a_half" if aligned else "_half"
        vec_sizes = VEC_SIZES if aligned else [''] + VEC_SIZES

        gen_test_global(suffix, 'float',  'half', vec_sizes, aligned);
        gen_test_global(suffix, 'double', 'half', vec_sizes, aligned);
        gen_test_local(suffix, 'float',  'half', vec_sizes, aligned);
        gen_test_local(suffix, 'double', 'half', vec_sizes, aligned);
        gen_test_private(suffix, 'float',  'half', vec_sizes, aligned);
        gen_test_private(suffix, 'double', 'half', vec_sizes, aligned);


if __name__ == '__main__':
    main()
