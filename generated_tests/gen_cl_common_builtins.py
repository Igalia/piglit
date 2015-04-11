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

from __future__ import print_function, division, absolute_import
import os

from genclbuiltins import gen, NEGNAN
from math import radians, degrees, pi

CLC_VERSION_MIN = {
    'clamp' : 10,
    'degrees' : 10,
    'max' : 10,
    'min' : 10,
    'mix' : 10,
    'radians' : 10,
    'step' : 10,
    'smoothstep' : 10,
    'sign' : 10
}

DATA_TYPES = ['float']

F = {
    'float' : 'float'
}

I = {
    'float' : 'int'
}

tests = {
    'clamp' : {
        'arg_types': [F, F, F, F],
        'function_type': 'tss',
        'values': [
            [0.5,  0.0, 0.0,  0.0, float("nan")], #Result
            [1.0, -0.5, 0.0,  0.0, float("nan")], #Arg0
            [0.0,  0.0, 0.0, -0.5, float("nan")], #Arg1
            [0.5,  0.5, 0.0,  0.5, float("nan")], #Arg2
        ]
    },
    'degrees' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [degrees(0.5), degrees(-0.5), 180.0, 0.0, 360, 1800.0, 18000, 90], #Result
            [0.5,                   -0.5, pi,    0.0, 2*pi, 10*pi, 100*pi, pi/2] #Arg0
        ]
    },
    'max' : {
        'arg_types': [F, F, F],
        'function_type': 'tss',
        'values': [
            [1.0,  0.0, 0.0,  0.0 ], #Result
            [1.0, -0.5, 0.0,  0.0 ], #Arg0
            [0.0,  0.0, 0.0, -0.5 ] #Arg1
        ]
    },
    'min' : {
        'arg_types': [F, F, F],
        'function_type': 'tss',
        'values': [
            [0.0, -0.5, 0.0, -0.5 ], #Result
            [1.0, -0.5, 0.0,  0.0 ], #Arg0
            [0.0,  0.0, 0.0, -0.5 ] #Arg1
        ]
    },
    'mix' : { #x + (y - x) * a
        'arg_types': [F, F, F, F],
        'function_type': 'tts',
        'values': [
            [0.5, -0.25, 0.0, -0.5, 1.0, 3.0, 10.0, float("nan"), float("nan"), float("nan")], #Result
            [1.0, -0.5,  0.0,  0.0, 1.0, 4.0, 15.0, 5.0, 4.0, float("nan")], #Arg0
            [0.0,  0.0,  0.0, -0.5, 2.0, 2.0, 10.0, -0.2, float("nan"), 1.5], #Arg1
            [0.5,  0.5,  0.0,  1.0, 0.0, 0.5,  1.0, float("nan"), 1.0, 0.0], #Arg2
        ]
    },
    'radians' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.5,                   -0.5, pi,    0.0, 2*pi, 10*pi, 100*pi, pi/2], #Result
            [degrees(0.5), degrees(-0.5), 180.0, 0.0, 360, 1800.0, 18000, 90] #Arg0
        ]
    },
    #TODO Add scalar combination (tst?)
    'step' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [0.0,  1.0, 1.0,  0.0, 1.0, 1.0], #Result
            [1.0, -0.5, 0.0,  0.0, 1.0, float("nan")], #Arg0
            [0.0,  0.0, 0.0, -0.5, float("nan"), 1.0] #Arg1
        ]
    },
    #TODO Add scalar combination (tst?)
    'smoothstep' : {
        'arg_types': [F, F, F, F],
        'function_type': 'ttt',
        'values': [
            [0.0,  0.0, 1.0,  1.0, 0.5,  0.896], #Result
            [0.0,  0.0, 0.0, -0.5, 0.0,  0.0], #Arg0
            [1.0,  1.0, 0.5,  0.0, 0.5,  0.5], #Arg1
            [-0.5, 0.0, 0.5,  1.0, 0.25, 0.4] #Arg2
        ]
    },
    'sign' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, -1.0, 0.0, -0.0, 0.0], #Result
            [0.5, -0.5, 0.0, -0.0, float("nan")] #Arg0
        ]
    }
}


def main():
    dirName = os.path.join("cl", "builtin", "common")

    testDefs = {}
    functions = sorted(tests.keys())
    for dataType in DATA_TYPES:
        for fnName in functions:
            testDefs[(dataType, fnName)] = tests[fnName]

    gen(DATA_TYPES, CLC_VERSION_MIN, functions, testDefs, dirName)


if __name__ == '__main__':
    main()
