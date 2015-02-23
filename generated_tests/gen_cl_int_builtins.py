from __future__ import print_function, division
import os
from genclbuiltins import gen, DATA_SIZES, MAX_VALUES, MAX, MIN, BMIN, BMAX, \
                          SMIN, SMAX, UMIN, UMAX, TYPE, SIZE, T, U, B

# Builtins is a data structure of the following:
#  builtins = {
#    '{data_type}': { # data type is any of [u]char, [u]short, [u]int, [u]long
#        '{builtin_function_name}': {
#            'arg_types': ['{data_type}', '{data_type}', ...],
#            'function_type': 'ttt'|'tss',
#                           ttt = all arguments are same-length vectors
#                           tss = all arguments are either same-length vectors,
#                                  or a vector followed by scalars
#            'values': [
#                [array of test output (arg0) values],
#                [array of arg1 values],
#                [array of arg2 values],
#                ...
#            ]
#        },
#        '{next_function}': {...},
#        ...
#    },
#    '{next_type}': {...},
#    ...
#  }
#
#  The builtins_generic, builtins_signed/unsigned are the same, but lack the
#  {datatype} layer

SIGNED_TYPES = ['char', 'short', 'int', 'long']
UNSIGNED_TYPES = ['uchar', 'ushort', 'uint', 'ulong']
DATA_TYPES = SIGNED_TYPES + UNSIGNED_TYPES


CLC_VERSION_MIN = {
    'abs': 10,
    'abs_diff': 10,
    'add_sat': 10,
    'hadd': 10,
    'rhadd': 10,
    'clz': 10,
    'clamp': 11,
    'mad_hi': 10,
    'mad_sat': 10,
    'max': 11,  # max/min are only same-size in CL1.0, but TSS in CL1.1
    'min': 11,
    'mul_hi': 10,
    'rotate': 10,
    'sub_sat': 10,
    'upsample': 10,
    'mad24': 10,
    'mul24': 10
}


def abs(val):
    if (val < 0):
        return val*-1
    return val


def add(val1, val2):
    return val1+val2


# Given a data type, return the next bigger type of given signedness.
def big(type):
    return B[type]


def clz(type, val):
    if (val < 0):
        return 0
    else:
        # Count the number of times that we can right shift before value = 0
        # then subtract that from (data_size - 1)
        count = 0
        while(val > 0):
            if (val > 0):
                val = val >> 1
                count = count + 1
        return DATA_SIZES[type] - count


def div(val1, val2):
    return val1 // val2


def mad_hi(x, y, z, type):
    res = (x*y) >> DATA_SIZES[type]
    res = res + z
    while (res > MAX_VALUES[type]):  # Emulate overflow... Necessary?
        res = res - (2**DATA_SIZES[type])
    return res


def mul(val1, val2):
    return val1 * val2


def mul_hi(x, y, type):
    res = (x*y) >> DATA_SIZES[type]
    return res

# def pop(val,type):
#     # TODO: Calculate number of non-zero bits in value (CLC 1.2)
#     return 0


def pow(val, pow):
    return val ** pow


def rotate_right(x, n, bits):
    # Find all bits that will wrap
    mask = (1 << n) - 1
    wrapped_bits = x & mask

    # sign extension needs to be masked out
    se_mask = (1 << (bits - n)) - 1
    unwrapped_bits = x >> n
    unwrapped_bits &= se_mask

    return unwrapped_bits | (wrapped_bits << (bits - n))


def rotate_left(x, n, bits):
    return rotate_right(x, bits - n, bits)


def rot(x, n, bits):
    if (n < 0):
        return rotate_right(x, -1*n, bits)
    else:
        return rotate_left(x, n, bits)


def sub(val1, val2):
    return val1-val2


# Tests which don't depend on the signedness or bit-width of the inputs
generic_tests = {
    'abs': {
        'arg_types': [U, T],
        'function_type': 'ttt',
        'values': [
            [0, 2, [abs, MIN], [abs, MAX]],
            [0, 2,        MIN,        MAX]
        ]
    },
    'abs_diff': {
        'arg_types': [U, T, T],
        'function_type': 'ttt',
        'values': [
            [0, 1, 1, UMAX, UMAX],
            [0, 1, 0,  MIN,  MAX],
            [0, 0, 1,  MAX,  MIN]
        ]
    },
    'add_sat': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [MAX,           MAX],
            [MAX, [sub, MAX, 1]],
            [64,             50]
        ]
    },
    'hadd': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [32, 0, 10, MAX, MIN,
             [div, [add, MAX, MIN], 2],
             [div, [add, MIN, MAX], 2]],
            [32, 1, 12, MAX, MIN, MAX, MIN],
            [33, 0,  8, MAX, MIN, MIN, MAX]
        ]
    },
    'clz': {
        'arg_types': [T, T],
        'function_type': 'ttt',
        'values': [
            [[clz, TYPE,   1],
             [clz, TYPE,  64],
             [clz, TYPE,   0],
             [clz, TYPE, MAX],
             [clz, TYPE, MIN]],
            [1, 64, 0, MAX, MIN]]
    },
    'clamp': {
        'arg_types': [T, T, T, T],
        'function_type': 'tss',
        'values': [
            [64, [div, MIN, 2],  1],
            [92,           MIN, 64],
            [0,  [div, MIN, 2],  0],
            [64,             0,  1]
        ]
    },
    'mad_hi': {
        'arg_types': [T, T, T, T],
        'function_type': 'ttt',
        'values': [
            [[mad_hi, [div, MAX, 2],   3,   1, TYPE],
             [mad_hi,           MIN,   2,   2, TYPE], 4, 1,
             [mad_hi,           MAX, MAX, MAX, TYPE],
             [mad_hi,           MIN, MIN, MIN, TYPE],
             [mad_hi,           MIN, MAX, MAX, TYPE],
             [mad_hi,           MAX,   2,   2, TYPE]],
            [[div, MAX, 2], MIN, 12, MAX, MAX, MIN, MIN, MAX],
            [            3,   2,  4,   1, MAX, MIN, MAX, 2],
            [            1,   2,  4,   1, MAX, MIN, MAX, 2]
        ]
    },
    'mad_sat': {
        'arg_types': [T, T, T, T],
        'function_type': 'ttt',
        'values': [
            [52, MAX, 93, 0, MAX, MAX],
            [12, MAX, 92, 0, MAX, MAX],
            [ 4,   1,  1, 0,   2, MAX],
            [ 4,   1,  1, 0,   2, MAX]
        ]
    },
    'max': {
        'arg_types': [T, T, T],
        'function_type': 'tss',
        'values': [
            [92,   2, 12, MAX,   1, MAX, MIN, MAX, MAX, 0],
            [92,   2, 12, MAX, MIN, MAX, MIN, MIN, MAX, 0],
            [ 2, MIN,  4,   1,   1, MAX, MIN, MAX, MIN, 0]
        ]
    },
    'min': {
        'arg_types': [T, T, T],
        'function_type': 'tss',
        'values': [
            [ 2,  1, MIN,  4,   1, MIN, MAX, MIN, MIN, 0, MAX],
            [92, 64,   2, 12, MAX, MIN, MAX, MIN, MIN, 0, MAX],
            [ 2,  1, MIN,  4,   1,   1, MAX, MIN, MAX, 0, MAX]
        ]
    },
    'mul_hi': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [0, 0, 0,
             [mul_hi, MAX, MAX, TYPE],
             [mul_hi, MIN, MIN, TYPE], 0,
             [mul_hi, MAX,   2, TYPE],
             [div, MIN, 2]
            ],
            [0, 92, MAX, MAX, MIN, 92, MAX, MIN],
            [0,  2,   1, MAX, MIN,  1,   2, MAX]
        ]
    },
    'rhadd': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [33, 1, 10],
            [32, 1, 12],
            [33, 0,  8]
        ]
    },
    'rotate': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [1, 8, 1, 2, 1],
            [1, 1, 1, 1, 1],
            [0, 3, SIZE,
             [add, SIZE,  1],
             [mul, SIZE, 10]]
        ]
    },
    'sub_sat': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [1, 25, MAX,   0,   0, MIN, MIN],
            [1, 57, MAX, MAX, MIN, MIN, [div,           MIN, 2]],
            [0, 32, MIN, MAX, MIN, MAX, [add, [div, MAX, 2], 1]]
        ]
    },
    'upsample': {
        'arg_types': [B, T, U],
        'function_type': 'ttt',
        'values': [
            [[pow,              2, SIZE],
             [add, [pow, 2, SIZE],    1],
             BMAX,              0,  MAX,
             [add, [pow, 2, SIZE],    7]],
            [1, 1,  MAX, 0,   0, 1],
            [0, 1, UMAX, 0, MAX, 7]
        ]
    }
}

# Any test that conceivably includes a negative number as input/output
signed_generic_tests = {
    'abs': {
        'arg_types': [U, T],
        'function_type': 'ttt',
        'values': [
            [ 1,  13],
            [-1, -13]
        ]
    },
    'abs_diff': {
        'arg_types': [U, T, T],
        'function_type': 'ttt',
        'values': [
            [1, 15],
            [0, -8],
            [1,  7]
        ]
    },
    'add_sat': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [  0, -2, [sub, MAX, 63], MIN,            MIN],
            [ 32, -8,            MAX, MIN, [add, MIN, 10]],
            [-32,  6,            -63, -12,            -50]
        ]
    },
    'clamp': {
        'arg_types': [T, T, T, T],
        'function_type': 'tss',
        'values': [
            [ -64,  0],
            [-128, -1],
            [ -64,  0],
            [   0,  1]
        ]
    },
    'mad_hi': {
        'arg_types': [T, T, T, T],
        'function_type': 'ttt',
        'values': [
            [MIN, -2],
            [ -1,  1],
            [MIN, -1],
            [MIN, -1]
        ]
    },
    'mad_sat': {
        'arg_types': [T, T, T, T],
        'function_type': 'ttt',
        'values': [
            [  0, MIN, MIN, MAX, MIN, -2],
            [ -1, MIN, MIN, MIN, MIN,  1],
            [MIN,   2,   1, MIN, MAX, -1],
            [MIN,   2,  -1, MIN, MAX, -1]
        ]
    },
    'max': {
        'arg_types': [T, T, T],
        'function_type': 'tss',
        'values': [
            [ -1,  1],
            [ -1,  1],
            [MIN, -1]
        ]
    },
    'min': {
        'arg_types': [T, T, T],
        'function_type': 'tss',
        'values': [
            [MIN, -1, MIN],
            [ -1,  1, MIN],
            [MIN, -1,  -1]
        ]
    },
    'mul_hi': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [  0 , -1,  -1, -1],
            [ -1, MIN, MIN,  1],
            [MIN,   2,   1, -1]
        ]
    },
    'rhadd': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [[-32], [-33], [-32]]
    },
    'rotate': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [MIN, MIN, [rot, 1, -3, SIZE], 1, MIN, [pow, 2, [sub, SIZE, 2]],
             MIN, [rot, -2, -1, SIZE]], [1, 1, 1, 1, 1, 1, 1, -2],
            [[sub, SIZE, 1], -1, -3, [mul, SIZE, -1],
             [mul, [add, SIZE, 1], -1],
             [mul, [add, SIZE, 2], -1], [sub, SIZE, 1], -1]
        ]
    },
    'sub_sat': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [MAX,          81],
            [[sub, MAX, 8], 1],
            [-32,         -80]
        ]
    },
    'upsample': {
        'arg_types': [B, T, U],
        'function_type': 'ttt',
        'values': [
            [  -1, [mul, [pow, 2, SIZE], -1]],
            [  -1,                       -1],
            [UMAX,                        0]
        ]
    }

}

# This list contains all numeric tests which never include negative integers
# that can't go into generic_tests.
unsigned_generic_tests = {
    'mad_sat': {
        'arg_types': [T, T, T, T],
        'function_type': 'ttt',
        'values': [
            [2,   MIN, MAX],
            [MIN, MIN, MIN],
            [2,   MIN, MAX],
            [2,   MIN, MAX]
        ]
    },
    'rotate': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [[div, [add, MAX, 1], 2], [div, [add, MAX, 1], 2]],
            [1, 1], [[sub, SIZE, 1], [sub, SIZE, 1]]
        ]
    },
}

# Hand-coded tests which are data type specific.
builtins = {
    'int': {
        'mad24': {
            'arg_types': [T, T, T, T],
            'function_type': 'ttt',
            'values': [
                [0, 2,  0, 520],
                [0, 1, -1,  32],
                [0, 1, -1,  16],
                [0, 1, -1,   8]
            ]
        },
        'mul24': {
            'arg_types': [T, T, T],
            'function_type': 'ttt',
            'values': [
                [0, 1,  1, 512, 4278190081],
                [0, 1, -1,  32,    2**23-1],
                [0, 1, -1,  16,    2**23-1]
            ]
        }
    },
    'uint': {
        'mad24': {
            'arg_types': [T, T, T, T],
            'function_type': 'ttt',
            'values': [
                [0, 2, 4278190080, 520],
                [0, 1,    2**24-1,  32],
                [0, 1,    2**24-1,  16],
                [0, 1,    2**24-1,   8]
            ]
        },
        'mul24': {
            'arg_types': [T, T, T],
            'function_type': 'ttt',
            'values': [
                [0, 1, 4261412865, 512],
                [0, 1,    2**24-1,  32],
                [0, 1,     2**24-1, 16]
            ]
        }
    }
}


# # # #  Define helper functions # # # #


def addTestValues(origDef, origValues):
    fnDef = dict(origDef)
    values = list(origValues)
    if (not 'values' in fnDef):
        fnDef['values'] = []
        for idx in range(0, len(values)):
            fnDef['values'].append(list(values[idx]))
    else:
        for arg in range(0, len(values)):
            fnDef['values'][arg] += values[arg]
    return fnDef


# Given a data type and function name, create one set of combined applicable
# test definitions.
def mergedTestDefinition(dataType, fnName):
    mergedTest = dict()

    testLists = [generic_tests]
    if (dataType in SIGNED_TYPES):
        testLists += [signed_generic_tests]
    if (dataType in UNSIGNED_TYPES):
        testLists += [unsigned_generic_tests]
    if (dataType in builtins):
        testLists += [builtins[dataType]]

    for testList in testLists:
        if (fnName in testList):
            fnDef = dict(testList[fnName])
            if (not 'arg_types' in mergedTest):
                mergedTest['arg_types'] = list(fnDef['arg_types'])
                mergedTest['function_type'] = fnDef['function_type']
            mergedTest = addTestValues(dict(mergedTest), list(fnDef['values']))
    return mergedTest


def getFnNames():
    fnNames = []
    fnNames += generic_tests.keys()
    fnNames += signed_generic_tests.keys()
    fnNames += unsigned_generic_tests.keys()
    for type in DATA_TYPES:
        if (type in builtins):
            fnNames += builtins[type].keys()

    # Get the sorted unique set of function names
    return sorted(list(set(fnNames)))


def main():
    dirName = os.path.join("cl", "builtin", "int")

    testDefs = {}
    functions = getFnNames()

    for dataType in DATA_TYPES:
        for fnName in functions:
            if (fnName is 'upsample' and
                    (dataType is 'long' or dataType is 'ulong')):
                continue
            # Merge all of the generic/signed/unsigned/custom test definitions
            testDefs[(dataType, fnName)] = mergedTestDefinition(dataType, fnName)

    gen(DATA_TYPES, CLC_VERSION_MIN, functions, testDefs, dirName)


if __name__ == '__main__':
    main()
