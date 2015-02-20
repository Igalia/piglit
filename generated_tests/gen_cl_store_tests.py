# Copyright 2013 Advanced Micro Devices, Inc.
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
#
# Authors: Tom Stellard <thomas.stellard@amd.com>
#
#

import os
import textwrap

from six.moves import range

from modules import utils

TYPES = ['char', 'uchar', 'short', 'ushort', 'int', 'uint', 'long', 'ulong', 'float', 'double']
VEC_SIZES = ['', '2', '4', '8', '16']

dirName = os.path.join("cl", "store")
utils.safe_makedirs(dirName)


def gen_array(size):
    return ' '.join([str(i) for i in range(size * 8)])


def ext_req(type_name):
    if type_name[:6] == "double":
        return "require_device_extensions: cl_khr_fp64"
    return ""


def print_config(f, type_name, addr_space):
    f.write(textwrap.dedent(("""
    [config]
    name: Store {type_name}
    program_source_file: store-kernels-{addr_space}.inc
    build_options: -D TYPE={type_name}
    dimensions: 1
    """ + ext_req(type_name))
    .format(type_name=type_name, addr_space=addr_space)))


def begin_test(type_name, addr_space):
    fileName = os.path.join(dirName, 'store-' + type_name + '-' + addr_space + '.program_test')
    print(fileName)
    f = open(fileName, 'w')
    print_config(f, type_name, addr_space)
    return f


def main():
    for t in TYPES:
        for s in VEC_SIZES:
            if s == '':
                size = 1
            else:
                size = int(s)
            type_name = t + s
            f = begin_test(type_name, 'global')
            f.write(textwrap.dedent("""
            [test]
            name: global address space
            global_size: 1 0 0
            kernel_name: store_global
            arg_out: 0 buffer {type_name}[8] {gen_array}
            arg_in:  1 buffer {type_name}[8] {gen_array}
            [test]
            name: global address space work items
            global_size: 8 0 0
            kernel_name: store_global_wi
            arg_out: 0 buffer {type_name}[8] {gen_array}
            arg_in:  1 buffer {type_name}[8] {gen_array}
            """.format(type_name=type_name, gen_array=gen_array(size))))

            f.close()

            f = begin_test(type_name, 'local')
            f.write(textwrap.dedent("""
            [test]
            name: local address space
            global_size: 8 0 0
            local_size:  8 0 0
            kernel_name: store_local
            arg_out: 0 buffer {type_name}[8] {gen_array}
            arg_in:  1 buffer {type_name}[8] {gen_array}
            """.format(type_name=type_name, gen_array=gen_array(size))))

            f.close()


if __name__ == '__main__':
    main()
