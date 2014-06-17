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
#          Aaron Watry  <awatry@gmail.com>
#

import os

from genclbuiltins import gen, TRUE, NEGNAN

CLC_VERSION_MIN = {
    'isnan' : 10,
    'isequal' : 10,
    'isgreater' : 10,
    'isgreaterequal' : 10,
    'isnotequal' : 10,
    'signbit' : 10
}

DATA_TYPES = ['float']

F = {
    'float' : 'float'
}

I = {
    'float' : 'int'
}

tests = {
    'isnan' : {
        'arg_types': [I, F],
        'function_type': 'ttt',
        'values': [
            [0,   1,            0,   0],            # Result
            [0.0, float("nan"), 1.0, float("inf") ] # Arg0
        ]
    },
    'isequal' : {
        'arg_types': [I, F, F],
        'function_type': 'ttt',
        'values': [
            [TRUE, 0,            TRUE, 0,            0,            TRUE,         0,    TRUE ],    # Result
            [0.0,  float("nan"), 1.0,  1,            float("nan"), float("inf"), 1.0,  123.0 ], # Arg0
            [0.0,  float("nan"), 1.0,  float("nan"), 1,            float("inf"), 0.5,  123.0 ]  # Arg1
        ]
    },
    'isgreater' : {
        'arg_types': [I, F, F],
        'function_type': 'ttt',
        'values': [
            [0,   0,            0,   0,            0,            0,            TRUE, 0],    # Result
            [0.0, float("nan"), 1.0, 1,            float("nan"), float("inf"), 1.0,  0.5 ], # Arg0
            [0.0, float("nan"), 1.0, float("nan"), 1,            float("inf"), 0.5,  1.0 ]  # Arg1
        ]
    },
    'isgreaterequal' : {
        'arg_types': [I, F, F],
        'function_type': 'ttt',
        'values': [
            [TRUE, 0,            TRUE, 0,            0,            TRUE,         TRUE, 0],    # Result
            [0.0,  float("nan"), 1.0,  1,            float("nan"), float("inf"), 1.0,  0.5 ], # Arg0
            [0.0,  float("nan"), 1.0,  float("nan"), 1,            float("inf"), 0.5,  1.0 ]  # Arg1
        ]
    },
    'isnotequal' : {
        'arg_types': [I, F, F],
        'function_type': 'ttt',
        'values': [
            [0,    TRUE,         0,    TRUE,         TRUE,         0,            TRUE, TRUE],    # Result
            [0.0,  float("nan"), 1.0,  1,            float("nan"), float("inf"), 1.0,  0.5 ], # Arg0
            [0.0,  float("nan"), 1.0,  float("nan"), 1,            float("inf"), 0.5,  1.0 ]  # Arg1
        ]
    },
    'signbit' : {
        'arg_types': [I, F],
        'function_type': 'ttt',
        'values': [
            [0,   TRUE,        0,   0, TRUE,          0,            TRUE, 0           , TRUE    ], # Result
            [0.0, float(0)*-1, 1.0, 1, float("-inf"), float("inf"), -1.0, float("nan"), NEGNAN ]  # Arg0
        ]
    }
}


def main():
    dirName = os.path.join("cl", "builtin", "relational")

    testDefs = {}
    functions = sorted(tests.keys())
    for dataType in DATA_TYPES:
        for fnName in functions:
            testDefs[(dataType, fnName)] = tests[fnName]

    gen(DATA_TYPES, CLC_VERSION_MIN, functions, testDefs, dirName)


main()
