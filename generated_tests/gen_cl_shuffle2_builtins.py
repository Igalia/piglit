# coding=utf-8
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
import itertools
import os
import random
import six
import textwrap

from modules import utils
from genclbuiltins import MAX_VALUES

TYPES = {
    'char': 'uchar',
    'uchar': 'uchar',
    'short': 'ushort',
    'ushort': 'ushort',
    'half': 'ushort',
    'int': 'uint',
    'uint': 'uint',
    'float': 'uint',
    'long': 'ulong',
    'ulong': 'ulong',
    'double': 'ulong'
}

VEC_SIZES = ['2', '4', '8', '16']
ELEMENTS = 8

DIR_NAME = os.path.join("cl", "builtin", "misc")


def gen_array(size, m):
    return [random.randint(0, m) for i in six.moves.range(size)]


def permute(data1, data2, mask, ssize, dsize):
    ret = []
    for i, m in enumerate(mask):
        src = data1 if (m % (2 * ssize)) < ssize else data2
        ret.append(src[(m % ssize) + ((i // dsize) * ssize)])
    return ret


def ext_req(type_name):
    if type_name[:6] == "double":
        return "require_device_extensions: cl_khr_fp64"
    if type_name[:4] == "half":
        return "require_device_extensions: cl_khr_fp16"
    return ""


def print_config(f, type_name, utype_name):
    f.write(textwrap.dedent(("""\
    /*!
    [config]
    name: shuffle2 {type_name} {utype_name}
    dimensions: 1
    """ + ext_req(type_name))
    .format(type_name=type_name, utype_name=utype_name)))


def begin_test(type_name, utype_name):
    fileName = os.path.join(DIR_NAME, 'builtin-shuffle2-{}-{}.cl'.format(type_name, utype_name))
    print(fileName)
    f = open(fileName, 'w')
    print_config(f, type_name, utype_name)
    return f


def main():
    random.seed(0)
    utils.safe_makedirs(DIR_NAME)

    for t, ut in six.iteritems(TYPES):
        f = begin_test(t, ut)
        for ss, ds in itertools.product(VEC_SIZES, VEC_SIZES):
            ssize = int(ss) * ELEMENTS
            dsize = int(ds) * ELEMENTS
            stype_name = t + ss
            dtype_name = t + ds
            utype_name = ut + ds
            data1 = gen_array(ssize, MAX_VALUES['ushort'])
            data2 = gen_array(ssize, MAX_VALUES['ushort'])
            mask = gen_array(dsize, MAX_VALUES[ut])
            perm = permute(data1, data2, mask, int(ss), int(ds))
            f.write(textwrap.dedent("""
            [test]
            name: shuffle2 {stype_name} {utype_name}
            global_size: {elements} 0 0
            kernel_name: test_shuffle2_{stype_name}_{utype_name}
            arg_out: 0 buffer {dtype_name}[{elements}] {perm}
            arg_in:  1 buffer {stype_name}[{elements}] {data1}
            arg_in:  2 buffer {stype_name}[{elements}] {data2}
            arg_in:  3 buffer {utype_name}[{elements}] {mask}
            """.format(stype_name=stype_name, utype_name=utype_name,
                       dtype_name=dtype_name, elements=ELEMENTS,
                       perm=' '.join([str(x) for x in perm]),
                       data1=' '.join([str(x) for x in data1]),
                       data2=' '.join([str(x) for x in data2]),
                       mask=' '.join([str(x) for x in mask]))))

        f.write(textwrap.dedent("""!*/"""))

        if t == "double":
            f.write(textwrap.dedent("""
            #pragma OPENCL EXTENSION cl_khr_fp64: enable
            """))
        if t == "half":
            f.write(textwrap.dedent("""
            #pragma OPENCL EXTENSION cl_khr_fp16: enable
            """))

        for ss, ds in itertools.product(VEC_SIZES, VEC_SIZES):
            type_name = t + ss
            utype_name = ut + ds
            f.write(textwrap.dedent("""
            kernel void test_shuffle2_{type_name}{ssize}_{utype_name}{dsize}(global {type_name}* out, global {type_name}* in1, global {type_name}* in2, global {utype_name}* mask) {{
                vstore{dsize}(shuffle2(vload{ssize}(get_global_id(0), in1), vload{ssize}(get_global_id(0), in2), vload{dsize}(get_global_id(0), mask)), get_global_id(0), out);
            }}
            """.format(type_name=t, utype_name=ut, ssize=ss, dsize=ds)))

        f.close()


if __name__ == '__main__':
    main()
