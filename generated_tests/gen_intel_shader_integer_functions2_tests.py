# coding=utf-8
#
# Copyright © 2018 Intel Corporation
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

from __future__ import print_function, division, absolute_import
import struct
import os
import numpy as np
import random

import six

from templates import template_file
from modules import utils

def generate_results_commutative(srcs, operator):
    """Generate results for an operator that is commutative.

    Commutative operators will only generate an upper-right triangular
    matrix of results, and the diagonal will be missing.
    """
    results = []
    for i in range(len(srcs) - 1):
        for j in range(i + 1, len(srcs)):
            results.append(operator(srcs[i], srcs[j]))

    return results


def generate_results_empty(unused1, unused2):
    """Some tests don't need any explicit results stored in the shader."""
    return []


def abs_isub32(_a, _b):
    a = np.int32(_a)
    b = np.int32(_b)

    return np.uint32(a - b) if a > b else np.uint32(b - a)


def abs_isub64(_a, _b):
    a = np.int64(_a)
    b = _b.astype(np.int64)

    return np.uint64(a - b) if a > b else np.uint64(b - a)


def abs_usub32(_a, _b):
    a = np.uint32(_a)
    b = np.uint32(_b)

    return a - b if a > b else b - a


def abs_usub64(_a, _b):
    a = np.uint64(_a)
    b = np.uint64(_b)

    return a - b if a > b else b - a


def absoluteDifference32_sources():
    srcs = []
    for x in range(0, 32, 4):
        srcs += [ -(0x80000000 >> x), -(0x7fffffff >> x) ]

    srcs += [-5, -3, -1, 0, 1, 3, 5]

    for x in range(32 - 4, 0, -4):
        srcs += [ 0x7fffffff >> x, 0x80000000 >> x ]

    srcs.append(0x7fffffff)

    # Some prime numbers requiring from 14- to 32-bits to store.  The last is
    # actually negative.
    srcs += [ 0x00002ff9,
              0x0017fff5,
              0x017ffff5,
              0x05fffffb,
              0x2ffffff5,
              0xbffffff5
    ]

    return srcs


def absoluteDifference64_sources():
    srcs = []
    for x in range(0, 64, 6):
        srcs += [ -(0x8000000000000000 >> x), -(0x7fffffffffffffff >> x) ]

    srcs += [-5, -3, -2, -1, 0, 1, 2, 3, 5]

    for x in range(64 - 4, 0, -6):
        srcs += [ 0x7fffffffffffffff >> x, 0x8000000000000000 >> x ]

    srcs.append(0x7fffffffffffffff)

    # Some prime numbers requiring from 33- to 64-bits to store.  The last is
    # actually negative.
    srcs += [ 0x000000017ffffffb,    # 33 bits
              0x00000017ffffffef,    # 37 bits
              0x0000017ffffffff3,    # 41 bits
              0x000017ffffffffff,    # 45 bits
              0x00017fffffffffe1,    # 49 bits
              0x0005ffffffffffdd,    # 51 bits
              0x0017fffffffffff3,    # 53 bits
              0x017fffffffffffb5,    # 57 bits
              0x037fffffffffffe5,    # 58 bits
              0x17ffffffffffffe1,    # 61 bits
              0x5fffffffffffff89,    # 63 bits
              0xbfffffffffffffe1,    # 64 bits
    ]

    assert len(srcs) == 64
    return [np.uint64(x) for x in srcs]


def countLeadingZeros_sources():
    sources=[]
    random.seed(0)

    for i in range(1024):
        num_zeros = i % 33

        if i < 33:
            sources.append(0xffffffff >> num_zeros)
        else:
            sources.append((random.randint(0, 0xffffffff) | (1 << 31)) >> num_zeros)

    return sources


def countTrailingZeros_sources():
    sources=[]
    random.seed(0)

    for i in range(1024):
        num_zeros = i % 33

        if i < 33:
            sources.append(0xffffffff << num_zeros)
        else:
            sources.append((random.randint(0, 0xffffffff) | 1) << num_zeros)

    return sources


FUNCS = {
    'absoluteDifference-int': {
        'input':      'int',
        'output':     'uint',
        'sources':    absoluteDifference32_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'absoluteDifference',
        'operator':   abs_isub32,
        'version':    '1.30',
        'extensions': None,
    },
    'absoluteDifference-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    absoluteDifference32_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'absoluteDifference',
        'operator':   abs_usub32,
        'version':    '1.30',
        'extensions': None,
    },
    'absoluteDifference-int64': {
        'input':      'int64_t',
        'output':     'uint64_t',
        'sources':    absoluteDifference64_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'absoluteDifference',
        'operator':   abs_isub64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'absoluteDifference-uint64': {
        'input':      'uint64_t',
        'output':     'uint64_t',
        'sources':    absoluteDifference64_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'absoluteDifference',
        'operator':   abs_usub64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'countLeadingZeros-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    countLeadingZeros_sources,
        'results':    generate_results_empty,
        'template':   'countLeadingZeros.shader_test.mako',
        'func':       'countLeadingZeros',
        'operator':   None,
        'version':    '1.30',
        'extensions': None,
    },
    'countTrailingZeros-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    countTrailingZeros_sources,
        'results':    generate_results_empty,
        'template':   'countLeadingZeros.shader_test.mako',
        'func':       'countTrailingZeros',
        'operator':   None,
        'version':    '1.30',
        'extensions': None,
    },
}


def main():
    """main function."""
    dirname = os.path.join('spec', 'intel_shader_integer_functions2',
                           'execution', 'built-in-functions')
    utils.safe_makedirs(dirname)

    for func, attrib in six.iteritems(FUNCS):

        TEMPLATE = template_file(os.path.basename(os.path.splitext(__file__)[0]),
                                 attrib['template'])

        for execution_stage in ('vs', 'fs'):
            filename = os.path.join(
                dirname, "{0}-{1}.shader_test".format(execution_stage, func))
            print(filename)

            extension_list = ["GL_INTEL_shader_integer_functions2"]
            if isinstance(attrib['extensions'], str):
                extension_list += [attrib['extensions']]
            elif attrib['extensions'] is not None:
                extension_list += attrib['extensions']

            with open(filename, 'w') as f:
                f.write(TEMPLATE.render_unicode(
                    execution_stage=execution_stage,
                    version=attrib['version'],
                    extensions=sorted(extension_list),
                    input_type=attrib['input'],
                    output_type=attrib['output'],
                    sources=attrib['sources'](),
                    results=attrib['results'](attrib['sources'](), attrib['operator']),
                    func=attrib['func']
                ))
    return

if __name__ == '__main__':
    main()
