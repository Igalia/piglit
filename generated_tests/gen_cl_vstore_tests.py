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
import textwrap
import random

from six.moves import range

from modules import utils

TYPES = ['char', 'uchar', 'short', 'ushort', 'int', 'uint', 'long', 'ulong', 'half', 'float', 'double']
VEC_SIZES = ['2', '4', '8', '16']

dirName = os.path.join("cl", "vstore")


def gen_array(size):
    random.seed(size)
    return ' '.join([str(random.randint(0, 255)) for i in range(size)])


def ext_req(type_name):
    if type_name[:6] == "double":
        return "require_device_extensions: cl_khr_fp64"
    if type_name[:6] == "half":
        return "require_device_extensions: cl_khr_fp16"
    return ""

def begin_test(suffix, type_name, addr_space):
    fileName = os.path.join(dirName, 'vstore'+ suffix + '-' + type_name + '-' + addr_space + '.cl')
    print(fileName)
    f = open(fileName, 'w')
    f.write(textwrap.dedent(("""
    /*!
    [config]
    name: Vector store{suffix} {type_name}2,4,8,16
    clc_version_min: 10

    dimensions: 1
    global_size: 1 0 0
    """ + ext_req(type_name))
    .format(type_name=type_name, addr_space=addr_space, suffix=suffix)))
    return f

def gen_test(suffix, t, mem_type, vec_sizes):
    f = begin_test(suffix, t, 'global')
    for s in vec_sizes:
        size = int(s) if s != '' else 1
        type_name = t + s
        f.write(textwrap.dedent("""
        [test]
        name: vector store{suffix} global {type_name}
        kernel_name: vstore{suffix}{n}_global
        arg_out: 0 buffer {mem_type}[{size}] 0 {gen_array}
        arg_in:  1 buffer {type_name}[1] {gen_array}

        [test]
        name: vector store{suffix} global offset {type_name}
        kernel_name: vstore{suffix}{n}_global_offset
        arg_out: 0 buffer {mem_type}[{offset_size}] {zeros} {gen_array}
        arg_in:  1 buffer {type_name}[1] {gen_array}
        """.format(type_name=type_name, mem_type=mem_type, size=size + 1,
                   zeros=("0 " * (size + 1)), offset_size=size*2 + 1, n=s,
                   gen_array=gen_array(size), suffix=suffix)))

    f.write(textwrap.dedent("""
    !*/
    """))
    if t == "double":
        f.write(textwrap.dedent("""
        #pragma OPENCL EXTENSION cl_khr_fp64: enable
        """))
    if t == "half":
        f.write(textwrap.dedent("""
        #pragma OPENCL EXTENSION cl_khr_fp16: enable
        """))
    for s in vec_sizes:
        type_name = t + s
        f.write(textwrap.dedent("""
        kernel void vstore{suffix}{n}_global(global {mem_type} *out,
                                     global {type_name} *in) {{
            {type_name} tmp = in[0];
            vstore{suffix}{n}(({type_name})0, 0, out);
            vstore{suffix}{n}(tmp, 0, out + 1);
        }}

        kernel void vstore{suffix}{n}_global_offset(global {mem_type} *out,
                                            global {type_name} *in) {{
            {type_name} tmp = ({type_name})0;
            vstore{suffix}{n}(tmp, 0, out);
            vstore{suffix}{n}(tmp, 0, out + 1);
            tmp = in[0];
            vstore{suffix}{n}(tmp, 1, out + 1);
        }}
        """.format(type_name=type_name, mem_type=mem_type, n=s, suffix=suffix)))

    f.close()


def main():
    utils.safe_makedirs(dirName)
    for t in TYPES:
        gen_test('', t, t, VEC_SIZES);

    gen_test('_half', 'float',  'half', [''] + VEC_SIZES);
    gen_test('_half', 'double', 'half', [''] + VEC_SIZES);

if __name__ == '__main__':
    main()
