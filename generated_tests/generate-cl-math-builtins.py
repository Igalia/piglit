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

from genclbuiltins import gen, NEGNAN
from math import acos, acosh, asin, asinh, atan, atan2, atanh, cos
from math import fabs, fmod, pi, pow, sin, sqrt, tan

CLC_VERSION_MIN = {
    'acos' : 10,
    'acosh' : 10,
    'asin' : 10,
    'asinh' : 10,
    'atan' : 10,
    'atan2' : 10,
    'atanh' : 10,
    'cbrt' : 10,
    'ceil' : 10,
    'copysign' : 10,
    'cos' : 10,
    'fabs' : 10,
    'floor' : 10,
    'fmod' : 10,
    'mix' : 10,
    'nextafter' : 10,
    'round' : 10,
    'sign' : 10,
    'sin' : 10,
    'tan' : 10,
    'sqrt' : 10,
    'trunc' : 10
}

DATA_TYPES = ['float']

F = {
    'float' : 'float'
}

tests = {
    'acos' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [ pi,  pi/2, 0.0, acos(0.12345), float("nan")], # Result
            [-1.0, 0.0,  1.0,      0.12345,  float("nan")]  # Arg0
        ],
        'tolerance' : 4
     },
    'acosh' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, acosh(1.12345), float("nan"), acosh(123456789.01234)], #Result
            [1.0,       1.12345,  float("nan"),       123456789.01234 ]  #Arg0
        ],
        'tolerance' : 4
     },
    'asin' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [-pi/2, 0.0, pi/2, asin(0.12345), float("nan")], # Result
            [-1.0,  0.0,  1.0,      0.12345,  float("nan")]  # Arg0
        ],
        'tolerance' : 4
     },
    'asinh' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, asinh(1.0), asinh(-1.12345), float("nan"), asinh(123456789.01234)], #Result
            [0.0, 1.0,              -1.12345,  float("nan"),       123456789.01234 ]  #Arg0
        ],
        'tolerance' : 4
     },
    'atan' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [atan(0.0), atan(0.12345), atan(3567147.0)], # Result
            [0.0,       0.12345,       3567147.0]# Arg0
        ],
        'tolerance' : 2
     },
    'atan2' : {
        'arg_types' : [F, F, F],
        'function_type': 'ttt',
        'values' : [
            [atan2(0.0, 0.0), atan2(1.2345, 10.0), atan2(35671470.0, 0.1)], # Result
            [0.0,             1.2345,              35671470.0            ], # Arg0
            [0.0,             10.0,                0.1                   ]  # Arg1
        ],
        'tolerance' : 6
     },
    'atanh' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, float("inf"), float("-inf"), float("nan"), atanh(0.123456789)], #Result
            [0.0, 1.0,              -1.0,      float("nan"),       0.123456789 ]  #Arg0
        ],
        'tolerance' : 4
     },
    'cbrt' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [3.0,  -1.0, float("nan"), float("inf"), 0.123456789**(1/3.0) ],
            [27.0, -1.0, float("nan"), float("inf"), 0.123456789 ]
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
    'copysign' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, -0.0, 1.0, -1.0, float("nan"), float("nan"), NEGNAN,       float("-inf"), float("inf") ], # Result
            [0.0,  0.0, 1.0, -1.0, float("nan"), -4.0,         float("nan"), float("inf"),  float("-inf") ], # Arg0
            [1.0, -1.0, 2.0, -2.0, float("nan"), float("nan"), -4.0,         -3.0,          float("inf") ], # Arg1
        ]
    },
    'cos' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0, 0.0,    -1.0, 0.0,        1.0,    cos(1.12345), -0.9258790228548379], # Result
            [0.0, pi / 2, pi,   3 * pi / 2, 2 * pi, 1.12345, pow(2,120)] # Arg0
        ],
        'tolerance' : 2
    },
    'fabs' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, pi/2,  pi,  0.0, float("inf"),  float("inf"), 1.12345 ], # Result
            [0.0, -pi/2, pi, -0.0, float("-inf"), float("inf"), -1.12345] # Arg0
        ],
        'tolerance' : 0
    },
    'floor' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, -1.0, 0.0, -0.0, float("nan"), -4.0,  1.0],
            [0.5, -0.5, 0.0, -0.0, float("nan"), -3.99, 1.5]
        ]
    },
    'fmod' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [float.fromhex("0x1.99998p-4"),  float("nan"), float.fromhex("-0x1.47aep-7"),   1.0, float("-nan"), float("nan")],
            [float.fromhex("0x1.466666p+2"), 0.0,          float.fromhex("-0x1p+2"),        1.0, 5.1,           3.0         ],
            [float.fromhex("0x1p-1"),        float("nan"), float.fromhex("-0x1.feb852p+1"), 1.5, 0.0,           float("inf")]
        ],
        'tolerance' : 0
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
    'tan' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, 1.0,  0.0, sqrt(3), -1.0,   tan(2.234567), float("nan") ], # Result
            [0.0, pi/4, pi,  pi/3,    3*pi/4, 2.234567 ,     float("nan") ], # Arg1
        ],
        'tolerance': 5
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
