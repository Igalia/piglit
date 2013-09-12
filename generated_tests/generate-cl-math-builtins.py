#!/usr/bin/env python
#
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

import os

from genclbuiltins import gen

CLC_VERSION_MIN = {
    'nextafter' : 10
}

DATA_TYPES = ['float']

F = {
    'float' : 'float'
}

tests = {
    'nextafter' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [1.401298e-45, -1.401298e-45, 1.00000011920928955078125, 0.999999940395355224609375, float("nan"), float("nan"), 5.0 ], # Result
            [0.0,           0.0         , 1.0, 1.0, float("nan"), 2.5, 5.0], # Arg0
            [1.0,          -1.0         , 2.0, 0.0, 3.4, float("nan"), 5.0], # Arg1
        ]
    }
}


def main():
    dirName = os.path.join("cl", "builtin", "math")

    testDefs = {}
    functions = sorted(tests.keys())
    for dataType in DATA_TYPES:
        for fnName in functions:
            testDefs[(dataType, fnName)] = tests[fnName]

    gen(DATA_TYPES, CLC_VERSION_MIN, functions, testDefs, dirName)


main()
