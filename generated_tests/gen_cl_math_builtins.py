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
from math import acos, acosh, asin, asinh, atan, atan2, atanh, cos, cosh, exp, expm1
from math import fabs, fmod, gamma, hypot, lgamma, log, log10, log1p, modf, pi, pow, sin, sinh, sqrt, tan, tanh

CLC_VERSION_MIN = {
    'acos' : 10,
    'acosh' : 10,
    'acospi' : 10,
    'asin' : 10,
    'asinh' : 10,
    'asinpi' : 10,
    'atan' : 10,
    'atan2' : 10,
    'atan2pi' : 10,
    'atanh' : 10,
    'atanpi': 10,
    'cbrt' : 10,
    'ceil' : 10,
    'copysign' : 10,
    'cos' : 10,
    'cosh' : 10,
    'cospi' : 10,
    'erf' : 10,
    'erfc' : 10,
    'exp' : 10,
    'exp10' : 10,
    'exp2' : 10,
    'expm1' : 10,
    'fabs' : 10,
    'fdim' : 10,
    'floor' : 10,
    'fma' : 10,
    'fmax' : 10,
    'fmin' : 10,
    'fmod' : 10,
    'fract' : 10,
    'frexp' : 10,
    'hypot' : 10,
    'ilogb' : 10,
    'ldexp' : 10,
    'lgamma' : 10,
    'lgamma_r' : 10,
    'log' : 10,
    'log10' : 10,
    'log1p' : 10,
    'log2' : 10,
    'logb' : 10,
    'nan' : 10,
    'mad' : 10,
    'maxmag' : 11,
    'minmag' : 11,
    'modf' : 10,
    'nextafter' : 10,
    'pow' : 10,
    'pown' : 10,
    'powr' : 10,
    'remainder' : 10,
    'remquo' : 10,
    'rint' : 10,
    'rootn' : 10,
    'round' : 10,
    'rsqrt' : 10,
    'sin' : 10,
    'sincos' : 10,
    'sinh' : 10,
    'sinpi' : 10,
    'sqrt' : 10,
    'tan' : 10,
    'tanh' : 10,
    'tanpi' : 10,
    'tgamma' : 10,
    'trunc' : 10
}

DATA_TYPES = ['float']

F = {
    'float' : 'float'
}

I = {
    'float' : 'int'
}

U = {
    'float' : 'uint'
}

M_PI_F = float.fromhex('0x1.921fb6p+1')

def quo(x, y):
    return int(round(x/y))

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
    'acospi' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [ 1,  1/2, 0.0, acos(0.12345) / pi, float("nan")], # Result
            [-1.0, 0.0,  1.0,      0.12345,  float("nan")]  # Arg0
        ],
        'tolerance' : 5
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
    'asinpi' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [-1/2, 0.0, 1/2, asin(0.12345)/pi, float("nan")], # Result
            [-1.0,  0.0,  1.0,      0.12345,  float("nan")]  # Arg0
        ],
        'tolerance' : 5
     },
    'atan' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [atan(0.0), atan(0.12345), atan(3567147.0)], # Result
            [0.0,       0.12345,       3567147.0]# Arg0
        ],
        'tolerance' : 5
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
    'atan2pi' : {
        'arg_types' : [F, F, F],
        'function_type': 'ttt',
        'values' : [
            [atan2(0.0, 0.0)/pi, atan2(1.2345, 10.0)/pi, atan2(35671470.0, 0.1)/pi], # Result
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
        'tolerance' : 5
     },
    'atanpi' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, -0.0, atan(1.02345)/pi, atan(-1.02345)/pi, float("nan"), 0.5,          -0.5          ],
            [0.0, -0.0, 1.02345,          -1.02345,          float("nan"), float("inf"), float("-inf") ]
        ],
        'tolerance' : 5
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
            # using libm cosf(3.0f * M_PI / 2.0f) == 0x1.99bc5cp-27
            # this is different form what python gives us
            [1.0, cos(M_PI_F / 2),    -1.0, float.fromhex('0x1.99bc5cp-27'),        1.0,    cos(1.12345), cos(7), cos(8), cos(pow(2,20)), cos(pow(2,24)), cos(pow(2,120)), float("nan")], # Result
            [0.0, M_PI_F / 2, pi,   3 * M_PI_F / 2, 2 * pi, 1.12345, 7, 8, pow(2,20), pow(2,24), pow(2,120), float("nan")] # Arg0
        ],
        'tolerance' : 4
    },
    'cosh' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0, cosh(0.123456789), float("inf"), float("inf"),  float("nan")],# Result
            [0.0, 0.123456789,       float("inf"), float("-inf"), float("nan")] # Arg0
        ],
        'tolerance' : 4
    },
    'cospi' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0, cos(pi*pi/2), cos(pi*3*pi/2), cos(2*pi*pi), cos(pi*1.12345), cos(pi*pow(2,20)), cos(pi*pow(2,24)), 1.0, float("nan")], # Result
            [0.0, pi / 2,       3 * pi / 2,     2 * pi,       1.12345        , pow(2,20),         pow(2,24),         pow(2,120),         float("nan")]  # Arg0
        ],
        'tolerance' : 4
    },
    'erf' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, 0.950004,        0.990005,        -0.994999475,  1.0, 1, -1], # Result
            [0.0, 1.960/sqrt(2.0), 2.576/sqrt(2.0), -2.807/sqrt(2.0), 11.1, float("inf"), float("-inf")]  # Arg0
        ],
        'tolerance' : 16
    },
    'erfc' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0, float.fromhex('0x1.9990c6p-5'), float.fromhex('0x1.4784aep-7'), 1.994999, 0.0, 0.0, 2.0], # Result
            [0.0, 1.960/sqrt(2.0), 2.576/sqrt(2.0), -2.807/sqrt(2.0), 11.1, float("inf"), float("-inf")]  # Arg0
        ],
        'tolerance' : 16
    },
    'exp' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0, exp(0.95), exp(pi), exp(-pi), float("inf"), float.fromhex('0x1.66fe8ap+4')], # Result
            [0.0, 0.95, pi, -pi, float("inf"), float.fromhex('0x1.8e2cp+1')]  # Arg0
        ],
        'tolerance' : 3
    },
    'exp10' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0,  10 ** 0.95, 10 ** pi, 10 ** -pi, float("inf"), float.fromhex('0x1.4298593c335e3p+10')], # Result
            [0.0, 0.95, pi, -pi, float("inf"), float.fromhex('0x1.8e2cp+1')]  # Arg0
        ],
        'tolerance' : 3
    },
    'exp2' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [1.0,  2 ** 0.95, 2 ** pi, 2 ** -pi, float("inf"), float.fromhex('0x1.146b7fd8431e3p+3')], # Result
            [0.0, 0.95, pi, -pi, float("inf"), float.fromhex('0x1.8e2cp+1')]  # Arg0
        ],
        'tolerance' : 3
    },
    'expm1' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, expm1(0.95), expm1(pi), expm1(-pi), float("inf"), float.fromhex('0x1.56fe8a160893ep+4')], # Result
            [0.0, 0.95, pi, -pi, float("inf"), float.fromhex('0x1.8e2cp+1')]  # Arg0
        ],
        'tolerance' : 3
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
    'fdim' : {
        'arg_types' : [F, F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, 0.75, 0.0, 0.0, float("inf"),  0.0,          float("nan"), float("nan"), float("nan"), 2.2469 ], # Result
            [0.3, 1.0,  pi,  0.0, float("inf"),  float("inf"), float("nan"), 1.0,          float("nan"), 1.12345 ], # Arg0
            [1.5, 0.25, pi, -0.0, float("-inf"), float("inf"), float("nan"), float("nan"), 1.0,          -1.12345] # Arg1
        ]
    },
    'floor' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, -1.0, 0.0, -0.0, float("nan"), -4.0,  1.0],
            [0.5, -0.5, 0.0, -0.0, float("nan"), -3.99, 1.5]
        ]
    },
    'fma' : {
        'arg_types': [F, F, F, F],
        'function_type': 'tss',
        'values': [
            [pi,   1.0, pi , -0.5, float("nan"), float("nan"), float("nan")], # Result
            [1.0,   pi, 0.0,  0.0, 1.0, float("nan"), float("nan")], # Arg0
            [pi,   0.0, pi,  -0.5, float("nan"), 1.0, float("nan")], # Arg1
            [0.0,  1.0, pi,  -0.5, float("nan"), 1.0, float("nan")]  # Arg2
        ]
    },
    'fmax' : {
        'arg_types': [F, F, F],
        'function_type': 'tss',
        'values': [
            [1.0,  0.0, 0.0,  0.0, 1.0, 1.0,          float("nan")], #Result
            [1.0, -0.5, 0.0,  0.0, 1.0, float("nan"), float("nan")], #Arg0
            [0.0,  0.0, 0.0, -0.5, float("nan"), 1.0, float("nan")] #Arg1
        ]
    },
    'fmin' : {
        'arg_types': [F, F, F],
        'function_type': 'tss',
        'values': [
            [0.0, -0.5, 0.0, -0.5, 1.0, 1.0,          float("nan")], #Result
            [1.0, -0.5, 0.0,  0.0, 1.0, float("nan"), float("nan")], #Arg0
            [0.0,  0.0, 0.0, -0.5, float("nan"), 1.0, float("nan")] #Arg1
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
    'fract' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        # For fract we have two outputs per address space.
        'values': [
            [float("nan"), 0.0,          0.5, 0.0, float.fromhex('0x1.33333p-2'), float.fromhex('0x1.fffffep-1') ], #fract
            [float("nan"), float("inf"),          1.0, 2.0, -2.0,                          -1.0], #floor
            [float("nan"), float("inf"), 1.5, 2.0,float.fromhex('-0x1.b33334p+0'), float.fromhex('-0x1.000242p-24')] #src0
        ],
        'num_out_args' : 2
    },
    'frexp' : {
        'arg_types': [F, I, F],
        'function_type': 'ttt',
        # For frexp we have two outputs per address space.
        'values': [
            [0.602783203125, 0.5, float("nan"), float("nan"),  float("inf"), float("-inf"), 0.0],
            [11,             1,   0,            0,             0,            0,             0],
            [1234.5,         1.0, float("nan"), float("-nan"), float("inf"), float("-inf"), 0.0]
        ],
       'num_out_args' : 2
    },
    'hypot' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [hypot(0, 1.0), hypot(3, 10.0), hypot(1, -3.0), hypot(10, 1234.5), hypot(2147483647, -1.0), float("inf")], # Result
            [0,      3,    1,     10, 2147483647,    2147483647], # Arg0
            [1.0, 10.0, -3.0, 1234.5,       -1.0, float("-inf")] # Arg1
        ],
        'tolerance' : 4
    },
    'ilogb' : {
        'arg_types': [I, F],
        'function_type': 'ttt',
        'values': [
            [0, 3, 1, 10, 2147483647, 2147483647],
            [1.0, 10.0, -3.0, 1234.5, float("inf"), float("-inf")]
        ],
        'tolerance' : 0
    },
    'ldexp' : {
        'arg_types': [F, F, I],
        'function_type': 'tss',
        'values': [
            [0.0, 4.0, 15.2, 1.75, float("nan"), float("inf")],
            [0.0, 1.0, 0.95, 3.5,  float("nan"), 1.12312312],
            [0,   2,   4,    -1,   1,            2031231231]
        ],
        'tolerance' : 0
    },
    'lgamma' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, lgamma(1.5), lgamma(0.5), float("nan"), lgamma(1.e-15), float("nan")], # Result
            [1.0, 1.5, 0.5,        0.0,           1.e-15,        float("nan")] # Arg
        ],
        'tolerance' : 16777216 # Specs say it's currently undefined
    },
    'lgamma_r' : {
        'arg_types': [F, I, F],
        'function_type': 'ttt',
        'values': [
            [0.0, lgamma(1.5), lgamma(0.5), float("nan"), lgamma(1.e-15), float("nan")], # Result0
            [1, -1, 1, 1, 1, 1], # Result1
            [1.0, 1.5, 0.5,        0.0,           1.e-15,        float("nan")] # Arg
        ],
        'tolerance' : 16777216, # Specs say it's currently undefined
        'num_out_args' : 2
    },
    'log' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [log(0.5), float("-inf"), log(1.e-15), float("nan")], #Result
            [0.5,      0.0,           1.e-15,      float("nan")]  #Arg0
        ],
        'tolerance' : 3
    },
    'log10' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [log10(0.5), float("-inf"), log10(1.e-15), float("nan")],
            [0.5,        0.0,           1.e-15,        float("nan")]
        ],
        'tolerance' : 3
    },
    'log1p' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [log1p(0.5), float("-inf"), log1p(1.e-15), float("nan")],
            [0.5,        -1.0,          1.e-15,        float("nan")]
        ],
        'tolerance' : 2
    },
    'log2' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [log(0.5, 2), float("-inf"), log(1.e-15, 2), float("nan")], #Result
            [0.5,         0.0,           1.e-15,         float("nan")]  #Arg0
        ],
        'tolerance' : 3
    },
    'logb' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0, 3, 1, 10, float("inf"), float("inf")], #Result
            [1.0, 10.0, -3.0, 1234.5, float("inf"), float("-inf")] #Arg0
        ],
        'tolerance' : 0
    },
    'mad' : {
        'arg_types': [F, F, F, F],
        'function_type': 'tss',
        'values': [
            [pi,   1.0, pi , -0.5, float("nan"), float("nan"), float("nan")], # Result
            [1.0,   pi, 0.0,  0.0, 1.0, float("nan"), float("nan")], # Arg0
            [pi,   0.0, pi,  -0.5, float("nan"), 1.0, float("nan")], # Arg1
            [0.0,  1.0, pi,  -0.5, float("nan"), 1.0, float("nan")]  # Arg2
        ],
        'tolerance' : 16777216 #infinite ULP
    },
    'maxmag' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, -0.5, 0.0, -0.5, 1.0, 1.0,          float("nan")], #Result
            [1.0, -0.5, 0.0,  0.0, 1.0, float("nan"), float("nan")], #Arg0
            [0.0,  0.0, 0.0, -0.5, float("nan"), 1.0, float("nan")] #Arg1
        ]
    },
    'minmag' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, -0.5, 0.0,  1.0, 1.0, 1.0,          float("nan")], #Result
            [1.0, -0.5, 1.0,  1.0, 1.0, float("nan"), float("nan")], #Arg0
            [0.0,  1.0, 0.0, -1.5, float("nan"), 1.0, float("nan")] #Arg1
        ]
    },
    'modf' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, modf(1.5)[0], modf(0.25)[0], 0.0, modf(1.e-15)[0], float("nan")], # Result0
            [1,   modf(1.5)[1], modf(0.25)[1], 0.0, modf(1.e-15)[1], float("nan")], # Result1
            [1.0, 1.5,          0.25,          0.0, 1.e-15,         float("nan")] # Arg
        ],
        'num_out_args' : 2
    },
# FIXME: kernel names are broken, and we cant really compare nans to see if the
# code made it
#    'nan' : {
#        'arg_types': [F, U],
#        'function_type': 'ttt',
#        'values': [
#            [float("nan"), float("nan"), float("nan")],
#            [0xdead, 0xadbeef, 0xdead]
#        ],
#        'tolerance' : 0
#    },
    'nextafter' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [1.401298e-45, -1.401298e-45, 1.00000011920928955078125, 0.999999940395355224609375, float("nan"), float("nan"), 5.0 ], # Result
            [0.0,           0.0         , 1.0, 1.0, float("nan"), 2.5, 5.0], # Arg0
            [1.0,          -1.0         , 2.0, 0.0, 3.4, float("nan"), 5.0], # Arg1
        ]
    },
    'pow' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [pow(0, 1.0), pow(-3, 10.0), pow(-11, -3.0), pow(1234.5, 10), pow(2147483647, -1.0), float("inf")], # Result
            [0,     -3,  -11, 1234.5, 2147483647,    2147483647], # Arg0
            [1.0, 10.0, -3.0,     10,       -1.0, float("-inf")] # Arg1
        ],
        'tolerance' : 16
    },
    'pown' : {
        'arg_types': [F, F, I],
        'function_type': 'ttt',
        'values': [
            [pow(1, 0), pow(10.0, 3), pow(-3.3, -4), pow(1234, 10), pow(-1, 2147483647), float("-inf")], # Result
            [1.0, 10.0, -3.3, 1234,       -1.0, float("-inf")], # Arg0
            [0,      3,   -4,   10, 2147483647,    2147483647] # Arg1
        ],
        'tolerance' : 16
    },
    'powr' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [pow(0, 1.0), pow(3, 10.0), pow(11, -3.0), pow(1234.5, 10), pow(2147483647, -1.0), 0.0], # Result
            [0,      3,   11, 1234.5, 2147483647,    2147483647], # Arg0
            [1.0, 10.0, -3.0,     10,       -1.0, float("-inf")] # Arg1
        ],
        'tolerance' : 16
    },
    'remainder' : {
        'arg_types': [F, F, F],
        'function_type': 'ttt',
        'values': [
            [float.fromhex("-0x1.ccccdp-1"), float.fromhex("0x1.ccccdp-1"),
             float.fromhex("-0x1.ccccdp-1"), float.fromhex("0x1.ccccdp-1"),
             0.0, -0.0, 5.1, float("-nan")
            ], # Result
            [ 5.1, -5.1,  5.1, -5.1, 0.0, -0.0, 5.1,          5.1], # Arg0
            [ 3.0,  3.0, -3.0, -3.0, 1.0,  1.0, float("inf"), 0.0], # Arg1
        ]
    },
    'remquo' : {
        'arg_types': [F, I, F, F],
        'function_type': 'ttt',
        'values': [
            [float.fromhex("-0x1.ccccdp-1"), float.fromhex("0x1.ccccdp-1"),
             float.fromhex("-0x1.ccccdp-1"), float.fromhex("0x1.ccccdp-1"),
             0.0, -0.0, 5.1, float("-nan")
            ], # Result
            [quo(5.1, 3.0), quo(-5.1, 3.0),  quo(5.1, -3.0), quo(-5.1, -3.0),
             quo(0.0, 1.0), quo(-0.0, 1.0), quo(5.1, float("inf")), 0], # Arg0
            [ 5.1, -5.1,  5.1, -5.1, 0.0, -0.0, 5.1,          5.1], # Arg0
            [ 3.0,  3.0, -3.0, -3.0, 1.0,  1.0, float("inf"), 0.0], # Arg1
        ],
        'num_out_args' : 2
    },
    'rint' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, -0.0, 1.0, -1.0, float("nan"), -4.0,  2.0, 0.0, 1.0],
            [0.5, -0.5, 0.6, -0.6, float("nan"), -3.99, 1.5, 0.4, 0.6]
        ]
    },
    'rootn' : {
        'arg_types': [F, F, I],
        'function_type': 'ttt',
        'values': [
            [pow(1, 1/2), pow(10.0, 1/3), float("nan"), pow(1234, 1/10), -1, float("-inf")], # Result
            [1.0, 10.0, -3.3, 1234,       -1.0, float("-inf")], # Arg0
            [2,      3,   -4,   10, 2147483647,    2147483647] # Arg1
        ],
        'tolerance' : 16
    },
    'round' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, -1.0, 0.0, -0.0, float("nan"), -4.0,  2.0, 0.0, 1.0],
            [0.5, -0.5, 0.0, -0.0, float("nan"), -3.99, 1.5, 0.4, 0.6]
        ]
    },
    'rsqrt' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, 1.0/2.0,  1/6.0, 1/2.5 , float("nan"), 1/4.0,  float("inf"), 1/sqrt(7.0), 1/sqrt(pi)], # Result
            [1.0, 4.0, 36.0, 6.25, float("nan"), 16.0, 0.0, 7.0, pi], # Arg1
        ],
        'tolerance': 2
    },
    'sin' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, 1.0,   0.0, -1.0,       0.0,    sin(2.234567), sin(7), sin(8), sin(pow(2,20)), sin(pow(2,24)), sin(pow(2,120)), float("nan")], # Result
            [0.0, pi / 2, pi, 3 * pi / 2, 2 * pi, 2.234567, 7, 8, pow(2,20), pow(2,24), pow(2,120), float("nan")] # Arg0
        ],
        'tolerance': 4
    },
    'sincos' : {
        'arg_types' : [F, F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, 1.0,   0.0, -1.0,       0.0,    sin(2.234567), sin(7), sin(8), sin(pow(2,20)), sin(pow(2,24)), sin(pow(2,120)), float("nan")], # Result0
            [1.0, 0.0,    -1.0, 0.0,        1.0,    cos(1.12345), cos(7), cos(8), cos(pow(2,20)), cos(pow(2,24)), cos(pow(2,120)), float("nan")], # Result1
            [0.0, pi / 2, pi, 3 * pi / 2, 2 * pi, 2.234567, 7, 8, pow(2,20), pow(2,24), pow(2,120), float("nan")] # Arg0
        ],
        'tolerance': 4,
        'num_out_args' : 2
    },
    'sinh' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, sinh(0.123456789), float("inf"), float("-inf"), float("nan")],# Result
            [0.0, 0.123456789,       float("inf"), float("-inf"), float("nan")] # Arg0
        ],
        'tolerance': 4
    },
    'sinpi' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, 0.0, sin(pi*pi/2), sin(pi*3*pi/2), sin(2*pi*pi),
             sin(pi*1.12345), sin(pi*7), sin(pi*8), sin(pi*pow(2,20)),
             sin(pi*pow(2,24)), sin(pi*pow(2,120)), float("nan")],#Result
            [0.0, 1.0, pi / 2,       3 * pi / 2,     2 * pi,
             1.12345        , 7,         8,         pow(2,20),
             pow(2,24),         pow(2,120),         float("nan")] #Arg0
        ],
        'tolerance' : 4
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
            [0.0, 1.0,  tan(M_PI_F), sqrt(3), -1.0,   tan(2.234567), float("nan") ], # Result
            [0.0, pi/4, M_PI_F,  pi/3,    3*pi/4, 2.234567 ,     float("nan") ], # Arg1
        ],
        'tolerance': 5
    },
    'tanh' : {
        'arg_types' : [F, F],
        'function_type': 'ttt',
        'values' : [
            [0.0, tanh(0.123456789), tanh(15.123456789), 1.0,          -1.0         , float("nan")],# Result
            [0.0, 0.123456789,       15.123456789,       float("inf"), float("-inf"), float("nan")] # Arg0
        ],
        'tolerance': 5
    },
    'tanpi' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [0.0, 1.0,  0.0, sqrt(3), -1.0,   tan(pi * 2.234567), float("nan") ], # Result
            [0.0, 1/4, 1,  1/3,    3/4, 2.234567 ,     float("nan") ], # Arg1
        ],
        'tolerance': 6
    },
    'tgamma' : {
        'arg_types': [F, F],
        'function_type': 'ttt',
        'values': [
            [1.0, gamma(1.5), gamma(0.5), float("nan"), gamma(1.e-15), float("nan")], # Result
            [1.0, 1.5, 0.5,        0.0,           1.e-15,        float("nan")] # Arg
        ],
        'tolerance' : 16
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


if __name__ == '__main__':
    main()
