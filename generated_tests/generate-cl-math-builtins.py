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
from math import atan, pi, sin, sqrt, cos

CLC_VERSION_MIN = {
    'atan' : 10,
    'ceil' : 10,
    'cos' : 10,
    'floor' : 10,
    'mix' : 10,
    'nextafter' : 10,
    'round' : 10,
    'sign' : 10,
    'sin' : 10,
    'sqrt' : 10,
    'trunc' : 10
}

DATA_TYPES = ['float']

F = {
    'float' : 'float'
}

tests = {
    'atan' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [atan(0.0), atan(0.12345), atan(3567147.0)], # Result
            [0.0,       0.12345,       3567147.0]# Arg0
        ],
        'tolerance' : 2
     },
    'ceil' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0,  0.0, 0.0, -0.0, float("nan"), -3.0],
            [0.5, -0.5, 0.0, -0.0, float("nan"), -3.99]
        ]
    },
    'cos' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0, 0.0,    -1.0, 0.0,        1.0,    cos(1.12345)], # Result
            [0.0, pi / 2, pi,   3 * pi / 2, 2 * pi, 1.12345] # Arg0
        ],
        'tolerance' : 2
    },
    'floor' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, -1.0, 0.0, -0.0, float("nan"), -4.0,  1.0],
            [0.5, -0.5, 0.0, -0.0, float("nan"), -3.99, 1.5]
        ]
    },
    'mix' : { #x + (y - x) * a
        'arg_types': [F, F, F, F],
        'function_type': 'tts',
        'values': [
            [float("nan"), float("nan"), 1.0, 3.0, 10.0  ], # Result
            [1.0         , 1.0,          1.0, 4.0, 15.0 ], # Arg0
            [2.0         , float("nan"), 2.0, 2.0, 10.0 ], # Arg1
            [float("nan"), 0.0,          0.0, 0.5, 1.0  ], # Arg2
        ]
    },
    'nextafter' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [1.401298e-45, -1.401298e-45, 1.00000011920928955078125, 0.999999940395355224609375, float("nan"), float("nan"), 5.0 ], # Result
            [0.0,           0.0         , 1.0, 1.0, float("nan"), 2.5, 5.0], # Arg0
            [1.0,          -1.0         , 2.0, 0.0, 3.4, float("nan"), 5.0], # Arg1
        ]
    },
    'round' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, -1.0, 0.0, -0.0, float("nan"), -4.0,  2.0, 0.0, 1.0],
            [0.5, -0.5, 0.0, -0.0, float("nan"), -3.99, 1.5, 0.4, 0.6]
        ]
    },
    'sign' : { # This is really a Common function but it uses the same types
               # as a lot of the math functions.
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, -1.0, 0.0, -0.0, 0.0],
            [0.5, -0.5, 0.0, -0.0, float("nan")]
        ]
    },
    'sin' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, 1.0,   0.0, -1.0,       0.0,    sin(2.234567)], # Result
            [0.0, pi / 2, pi, 3 * pi / 2, 2 * pi, 2.234567] # Arg0
        ],
        'tolerance': 2
    },
    'sqrt' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, 2.0,  6.0, 2.5 , float("nan"), 4.0,  sqrt(0.0), sqrt(7.0), sqrt(pi)], # Result
            [1.0, 4.0, 36.0, 6.25, float("nan"), 16.0, 0.0, 7.0, pi], # Arg1
        ],
        'tolerance': 3
    },
    'trunc' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, -0.0, 0.0, -0.0, float("nan"), -3.0,  1.0],
            [0.5, -0.5, 0.0, -0.0, float("nan"), -3.99, 1.5]
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
