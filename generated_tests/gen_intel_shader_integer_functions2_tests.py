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


def generate_results_commutative_with_diagonal(srcs, operator):
    """Generate results for an operator that is commutative.

    Commutative operators will only generate an upper-right triangular
    matrix of results, but the diagonal must also be explicitly stored.
    """
    results = []
    for i in range(len(srcs)):
        for j in range(i, len(srcs)):
            results.append(operator(srcs[i], srcs[j]))

    return results


def generate_results_without_diagonal(srcs, operator):
    """Generate full matrix of results without the diagonal."""
    results = []
    for i in range(len(srcs)):
        for j in range(len(srcs)):
            if i != j:
                results.append(operator(srcs[i], srcs[j]))

    return results


def generate_results_empty(unused1, unused2):
    """Some tests don't need any explicit results stored in the shader."""
    return []


def abs_isub32(_a, _b):
    a = np.int32(np.uint32(_a))
    b = np.int32(np.uint32(_b))

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


def iadd_sat32(_a, _b):
    a = np.int32(np.uint32(_a))
    b = np.int32(np.uint32(_b))

    if a > 0:
        if b > (np.iinfo(np.int32).max - a):
            return np.iinfo(np.int32).max
    else:
        if b < (np.iinfo(np.int32).min - a):
            return np.iinfo(np.int32).min

    return a + b


def uadd_sat32(_a, _b):
    a = np.uint32(_a)
    b = np.uint32(_b)

    if b > (np.iinfo(np.uint32).max - a):
        return np.iinfo(np.uint32).max

    return a + b


def iadd_sat64(_a, _b):
    a = np.int64(_a)
    b = np.int64(_b)

    if a > 0:
        if b > (np.iinfo(np.int64).max - a):
            return np.iinfo(np.int64).max
    else:
        if b < (np.iinfo(np.int64).min - a):
            return np.iinfo(np.int64).min

    return a + b


def uadd_sat64(_a, _b):
    a = np.uint64(_a)
    b = np.uint64(_b)

    if b > (np.iinfo(np.uint64).max - a):
        return np.iinfo(np.uint64).max

    return a + b


def isub_sat32(a, b):
    r = np.int64(np.int32(a)) - np.int64(np.int32(b))

    if r > np.int64(0x07fffffff):
        return np.int32(0x7fffffff)

    if r < np.int64(-0x080000000):
        return np.int32(-0x80000000)

    return np.int32(r)


def usub_sat32(_a, _b):
    a = np.uint32(_a)
    b = np.uint32(_b)

    return a - b if a > b else np.uint32(0)


def isub_sat64(_a, _b):
    a = np.int64(_a)
    b = np.int64(_b)

    if a >= 0:
        if (a - np.iinfo(np.int64).max) > b:
            return np.iinfo(np.int64).max
    elif b >= 0:
        if a < (np.iinfo(np.int64).min + b):
            return np.iinfo(np.int64).min

    return a - b


def usub_sat64(_a, _b):
    a = np.uint64(_a)
    b = np.uint64(_b)

    return a - b if a > b else np.uint64(0)


def u_hadd32(_a, _b):
    a = np.uint32(_a)
    b = np.uint32(_b)

    return (a >> 1) + (b >> 1) + ((a & b) & 1)


def s_hadd32(_a, _b):
    a = np.int32(np.uint32(_a))
    b = np.int32(np.uint32(_b))

    return (a >> 1) + (b >> 1) + ((a & b) & 1)


def u_hadd64(_a, _b):
    a = np.uint64(_a)
    b = np.uint64(_b)

    return (a >> np.uint64(1)) + (b >> np.uint64(1)) + ((a & b) & np.uint64(1))


def s_hadd64(_a, _b):
    a = np.int64(_a)
    b = np.int64(_b)

    return (a >> np.int64(1)) + (b >> np.int64(1)) + ((a & b) & np.int64(1))


def u_rhadd32(_a, _b):
    a = np.uint32(_a)
    b = np.uint32(_b)

    return (a >> 1) + (b >> 1) + ((a | b) & 1)


def s_rhadd32(_a, _b):
    a = np.int32(np.uint32(_a))
    b = np.int32(np.uint32(_b))

    return (a >> 1) + (b >> 1) + ((a | b) & 1)


def u_rhadd64(_a, _b):
    a = np.uint64(_a)
    b = np.uint64(_b)

    return (a >> np.uint64(1)) + (b >> np.uint64(1)) + ((a | b) & np.uint64(1))


def s_rhadd64(_a, _b):
    a = np.int64(_a)
    b = np.int64(_b)

    return (a >> np.int64(1)) + (b >> np.int64(1)) + ((a | b) & np.int64(1))


def imul_32x16(a, b):
    return np.int32(a) * ((np.int32(b) << 16) >> 16)


def umul_32x16(a, b):
    return np.uint32(np.uint32(a) * (np.uint32(b) & 0x0000ffff))


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


def addSaturate_int32_sources():
    srcs = [0, 1, -1, 2, 3, 0x40000000, 0x7fffffff, -0x7fffffff, -0x80000000 ]

    random.seed(0)
    for i in range(2, 32):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    for i in range(4):
        srcs.append(random.randint(-0x7ffffffe, -2))

    assert len(srcs) == 43
    return srcs


def addSaturate_uint32_sources():
    srcs = [0, 1, 2, 3, 0x40000000, 0x7fffffff, 0x80000000, 0xf0f0f0f0, 0xff00ff00 ]

    random.seed(0)
    for i in range(2, 32):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    for i in range(43 - len(srcs)):
        srcs.append(random.randint(-0x7ffffffe, -2))

    assert len(srcs) == 43
    return srcs


def addSaturate_int64_sources():
    srcs = [0, 1, -1, 2, 3, 0x4000000000000000, 0x7fffffffffffffff, -0x7fffffffffffffff, -0x8000000000000000 ]

    random.seed(0)
    for i in range(16, 64):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    while len(srcs) < 62:
        srcs.append(random.randint(-0x7ffffffffffffffe, -2))

    assert len(srcs) == 62
    return [np.int64(np.uint64(x)) for x in srcs]


def addSaturate_uint64_sources():
    srcs = [0, 1, 2, 3, 0x4000000000000000, 0x7fffffffffffffff, 0x8000000000000000, 0xf0f0f0f0f0f0f0f0, 0xff00ff00ff00ff00 ]

    random.seed(0)
    for i in range(16, 64):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    while len(srcs) < 61:
        srcs.append(random.randint(0, 0xffffffffffffffff))

    srcs.append(np.uint64(0xdeadbeefdeadbeef))

    assert len(srcs) == 62
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


def multiply32x16_int32_sources():
    srcs = [0, 1, -1, np.int32(-0x80000000), -0x7fffffff, 0x7fffffff ]

    random.seed(0)
    for i in range(2, 32, 3):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    srcs.append(random.randint(0, 1 << 30) | (1 << 30))

    for i in range(2, 32, 3):
        srcs.append(-(random.randint(0, 1 << i) | (1 << i)))

    srcs.append(-(random.randint(0, 1 << 30) | (1 << 30)))

    while len(srcs) < 512:
        srcs.append(random.randint(-0x80000000, 0x7fffffff))

    return srcs


def subtractSaturate_int32_sources():
    srcs = [0, 1, -1, np.int32(-0x80000000), -0x7fffffff, 0x7fffffff ]

    random.seed(0)
    for i in range(2, 32, 3):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    srcs.append(random.randint(0, 1 << 30) | (1 << 30))

    for i in range(2, 32, 3):
        srcs.append(-(random.randint(0, 1 << i) | (1 << i)))

    srcs.append(-(random.randint(0, 1 << 30) | (1 << 30)))

    while len(srcs) < 32:
        srcs.append(random.randint(-0x80000000, 0x7fffffff))

    assert len(srcs) == 32
    return [np.int32(x) for x in srcs]


def subtractSaturate_uint32_sources():
    srcs = [0, 1, 0xf0f0f0f0 ]

    random.seed(0)
    for i in range(2, 31):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    assert len(srcs) == 32
    return srcs


def subtractSaturate_int64_sources():
    srcs = [0, 1, -1, -0x8000000000000000, -0x7fffffffffffffff, 0x7fffffffffffffff ]

    random.seed(0)
    for i in range(2, 32, 3):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    srcs.append(random.randint(0, 1 << 30) | (1 << 30))

    for i in range(16, 64, 3):
        srcs.append(-(random.randint(0, 1 << i) | (1 << i)))

    srcs.append(-(random.randint(0, 1 << 30) | (1 << 30)))

    while len(srcs) < 45:
        srcs.append(random.randint(-0x8000000000000000, 0x7fffffffffffffff))

    assert len(srcs) == 45
    return [np.int64(x) for x in srcs]


def subtractSaturate_uint64_sources():
    srcs = [0, 1, 0xf0f0f0f0f0f0f0f0 ]

    random.seed(0)
    for i in range(22, 64):
        srcs.append(random.randint(0, 1 << i) | (1 << i))

    assert len(srcs) == 45
    return srcs


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
    'addSaturate-int': {
        'input':      'int',
        'output':     'int',
        'sources':    addSaturate_int32_sources,
        'results':    generate_results_commutative_with_diagonal,
        'template':   'addSaturate.shader_test.mako',
        'func':       'addSaturate',
        'operator':   iadd_sat32,
        'version':    '1.30',
        'extensions': None,
    },
    'addSaturate-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    addSaturate_uint32_sources,
        'results':    generate_results_commutative_with_diagonal,
        'template':   'addSaturate.shader_test.mako',
        'func':       'addSaturate',
        'operator':   uadd_sat32,
        'version':    '1.30',
        'extensions': None,
    },
    'addSaturate-int64': {
        'input':      'int64_t',
        'output':     'int64_t',
        'sources':    addSaturate_int64_sources,
        'results':    generate_results_commutative_with_diagonal,
        'template':   'addSaturate.shader_test.mako',
        'func':       'addSaturate',
        'operator':   iadd_sat64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'addSaturate-uint64': {
        'input':      'uint64_t',
        'output':     'uint64_t',
        'sources':    addSaturate_uint64_sources,
        'results':    generate_results_commutative_with_diagonal,
        'template':   'addSaturate.shader_test.mako',
        'func':       'addSaturate',
        'operator':   uadd_sat64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'average-int': {
        'input':      'int',
        'output':     'int',
        'sources':    absoluteDifference32_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'average',
        'operator':   s_hadd32,
        'version':    '1.30',
        'extensions': None,
    },
    'average-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    absoluteDifference32_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'average',
        'operator':   u_hadd32,
        'version':    '1.30',
        'extensions': None,
    },
    'average-int64': {
        'input':      'int64_t',
        'output':     'int64_t',
        'sources':    absoluteDifference64_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'average',
        'operator':   s_hadd64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'average-uint64': {
        'input':      'uint64_t',
        'output':     'uint64_t',
        'sources':    absoluteDifference64_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'average',
        'operator':   u_hadd64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
        },
    'averageRounded-int': {
        'input':      'int',
        'output':     'int',
        'sources':    absoluteDifference32_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'averageRounded',
        'operator':   s_rhadd32,
        'version':    '1.30',
        'extensions': None,
    },
    'averageRounded-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    absoluteDifference32_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'averageRounded',
        'operator':   u_rhadd32,
        'version':    '1.30',
        'extensions': None,
    },
    'averageRounded-int64': {
        'input':      'int64_t',
        'output':     'int64_t',
        'sources':    absoluteDifference64_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'averageRounded',
        'operator':   s_rhadd64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'averageRounded-uint64': {
        'input':      'uint64_t',
        'output':     'uint64_t',
        'sources':    absoluteDifference64_sources,
        'results':    generate_results_commutative,
        'template':   'absoluteDifference.shader_test.mako',
        'func':       'averageRounded',
        'operator':   u_rhadd64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'multiply32x16-int': {
        'input':      'int',
        'output':     'int',
        'sources':    multiply32x16_int32_sources,
        'results':    generate_results_empty,
        'template':   'multiply32x16.shader_test.mako',
        'func':       'multiply32x16',
        'operator':   imul_32x16,
        'version':    '1.30',
        'extensions': None,
    },
    'multiply32x16-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    multiply32x16_int32_sources,
        'results':    generate_results_empty,
        'template':   'multiply32x16.shader_test.mako',
        'func':       'multiply32x16',
        'operator':   umul_32x16,
        'version':    '1.30',
        'extensions': None,
    },
    'subtractSaturate-int': {
        'input':      'int',
        'output':     'int',
        'sources':    subtractSaturate_int32_sources,
        'results':    generate_results_without_diagonal,
        'template':   'subtractSaturate.shader_test.mako',
        'func':       'subtractSaturate',
        'operator':   isub_sat32,
        'version':    '1.30',
        'extensions': None,
    },
    'subtractSaturate-uint': {
        'input':      'uint',
        'output':     'uint',
        'sources':    subtractSaturate_uint32_sources,
        'results':    generate_results_without_diagonal,
        'template':   'subtractSaturate.shader_test.mako',
        'func':       'subtractSaturate',
        'operator':   usub_sat32,
        'version':    '1.30',
        'extensions': None,
    },
    'subtractSaturate-int64': {
        'input':      'int64_t',
        'output':     'int64_t',
        'sources':    subtractSaturate_int64_sources,
        'results':    generate_results_without_diagonal,
        'template':   'subtractSaturate.shader_test.mako',
        'func':       'subtractSaturate',
        'operator':   isub_sat64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
    },
    'subtractSaturate-uint64': {
        'input':      'uint64_t',
        'output':     'uint64_t',
        'sources':    subtractSaturate_uint64_sources,
        'results':    generate_results_without_diagonal,
        'template':   'subtractSaturate.shader_test.mako',
        'func':       'subtractSaturate',
        'operator':   usub_sat64,
        'version':    '4.00',  # GL_ARB_gpu_shader_int64 requires 4.0.
        'extensions': 'GL_ARB_gpu_shader_int64',
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
