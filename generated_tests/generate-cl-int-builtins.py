#!/usr/bin/env python

import os

#Builtins is a data structure of the following:
# builtins = {
#   '{data_type}': { #data type is any of [u]char, [u]short, [u]int, [u]long
#       '{builtin_function_name}': {
#           'arg_types': ['{data_type}', '{data_type}', ...],
#           'function_type': 'ttt'|'tss',
#                           #ttt = all arguments are same-length vectors
#                           #tss = all arguments are either same-length vectors,
#                                  or a vector followed by scalars
#           'values': [
#               [array of test output (arg0) values],
#               [array of arg1 values],
#               [array of arg2 values],
#               ...
#           ]
#       },
#       '{next_function}': {...},
#       ...
#   },
#   '{next_type}': {...},
#   ...
# }
#
# The builtins_generic, builtins_signed/unsigned are the same, but lack the
# {datatype} layer

#Define placeholders to reduce magic number usage
MAX = 'MAX_VAL'
MIN = 'MIN_VAL'

SIGNED_TYPES = ['char', 'short', 'int', 'long']
UNSIGNED_TYPES = ['uchar', 'ushort', 'uint', 'ulong']
DATA_TYPES = SIGNED_TYPES + UNSIGNED_TYPES
DATA_SIZES = {
    'char'  : 8,
    'uchar' : 8,
    'short' : 16,
    'ushort': 16,
    'int'   : 32,
    'uint'  : 32,
    'long'  : 64,
    'ulong' : 64
}

#By default, just test what is part of the CL1.1 spec, leave vec3 for later
#VEC_WIDTHS = (2, 3, 4, 8, 16)
VEC_WIDTHS = (2, 4, 8, 16)
#ALL_WIDTHS = [1, 2, 3, 4, 8, 16]
ALL_WIDTHS = [1, 2, 4, 8, 16]

MIN_VALUES = {
    'char'   : -128,
    'uchar'  : 0,
    'short'  : -32768,
    'ushort' : 0,
    'int'    : -2147483648,
    'uint'   : 0,
    'long'   : -9223372036854775808,
    'ulong'  : 0
}

MAX_VALUES = {
    'char'   : 127,
    'uchar'  : 255,
    'short'  : 32767,
    'ushort' : 65535,
    'int'    : 2147483647,
    'uint'   : 4294967295,
    'long'   : 9223372036854775807,
    'ulong'  : 18446744073709551615
}

#Identity type list
T = {
    'char'  : 'char',
    'uchar' : 'uchar',
    'short' : 'short',
    'ushort': 'ushort',
    'int'   : 'int',
    'uint'  : 'uint',
    'long'  : 'long',
    'ulong' : 'ulong'
}
#Signed type for each type
SIGNED = {
    'char'  : 'char',
    'uchar' : 'char',
    'short' : 'short',
    'ushort': 'short',
    'int'   : 'int',
    'uint'  : 'int',
    'long'  : 'long',
    'ulong' : 'long'
}
#Unsigned type for each source type
U = {
    'char'  : 'uchar',
    'uchar' : 'uchar',
    'short' : 'ushort',
    'ushort': 'ushort',
    'int'   : 'uint',
    'uint'  : 'uint',
    'long'  : 'ulong',
    'ulong' : 'ulong'
}
#Next larger type with same signedness
B = {
    'char': 'short',
    'uchar': 'ushort',
    'short': 'int',
    'ushort': 'uint',
    'int': 'long',
    'uint': 'ulong',
}

BMIN = 'min_for_larger_type'
BMAX = 'max_for_larger_type'
SMIN = 'signed_min_for_type'
SMAX = 'signed_max_for_type'
UMIN = 'unsigned_min_for_type'
UMAX = 'unsigned_max_for_type'
TYPE = 'TYPE'
SIZE = 'SIZE'

CLC_VERSION_MIN = {
    'abs' : 10,
    'abs_diff' : 10,
    'add_sat' : 10,
    'hadd' : 10,
    'rhadd' : 10,
    'clz' : 10,
    'clamp' : 11,
    'mad_hi' : 10,
    'mad_sat' : 10,
    'max' : 11, #max/min are only same-size in CL1.0, but TSS in CL1.1
    'min' : 11,
    'mul_hi' : 10,
    'rotate' : 10,
    'sub_sat' : 10,
    'upsample' : 10,
    'mad24' : 10,
    'mul24' : 10
}

def abs(val):
    if (val < 0):
        return val*-1
    return val

def add(val1, val2):
    return val1+val2

#Given a data type, return the next bigger type of given signedness.
def big(type):
    return B[type]

def clz(type, val):
    if (val < 0):
        return 0
    else:
        #Count the number of times that we can right shift before value = 0 then
        #subtract that from (data_size - 1)
        count=0
        while(val > 0):
            if (val > 0):
                val = val >> 1
                count = count + 1
        return DATA_SIZES[type] - count

def div(val1, val2):
    return val1 / val2

def mad_hi(x, y, z, type):
    res = (x*y) >> DATA_SIZES[type]
    res = res + z
    while (res > MAX_VALUES[type]): #Emulate overflow... Necessary?
        res = res - (2**DATA_SIZES[type])
    return res

def mul(val1, val2):
    return val1 * val2

def mul_hi(x,y,type):
    res = (x*y) >> DATA_SIZES[type]
    return res

#def pop(val,type):
#    #TODO: Calculate number of non-zero bits in value (CLC 1.2)
#    return 0

def pow(val,pow):
    return val ** pow

def rotate_right(x, n, bits):
    mask = (2L**n) - 1
    mask_bits = x & mask
    return (x >> n) | (mask_bits << (bits - n))

def rotate_left(x, n, bits):
    return rotate_right(x, bits - n, bits)

def rot(x, n, bits):
    if (n < 0):
        return rotate_right(x, -1*n, bits)
    else:
        return rotate_left(x, n, bits)

def sub(val1, val2):
    return val1-val2

def getValue(type, val):
    #Check if val is a str, list, or value
    if (isinstance(val, str)):
        if (val == MIN):
            return MIN_VALUES[type]
        elif (val == MAX):
            return MAX_VALUES[type]
        elif (val == BMIN):
            return MIN_VALUES[B[type]]
        elif (val == BMAX):
            return MAX_VALUES[B[type]]
        elif (val == SMIN):
            return MIN_VALUES[SIGNED[type]]
        elif (val == SMAX):
            return MAX_VALUES[SIGNED[type]]
        elif (val == UMIN):
            return MIN_VALUES[U[type]]
        elif (val == UMAX):
            return MAX_VALUES[U[type]]
        elif (val == TYPE):
            return type
        elif (val == SIZE):
            return DATA_SIZES[type]
        else:
            print('Unknown string value: '+val+'\n')
    elif (isinstance(val, list)):
        #The list should be of the format: [op, arg1, ... argN] where op is a Fn
        #ref and arg[1-n] are either MIN/MAX or numbers (They could be nested
        #lists). The exception for arg1 is TYPE, which means to substitute the
        #data type

        #Evaluate the value of the requested function and arguments
        #TODO: Change to varargs calls after unshifting the first list element
        if (len(val) == 2):
            return (val[0])(getValue(type,val[1]))
        elif (len(val) == 3):
            return (val[0])(getValue(type,val[1]), getValue(type, val[2]))
        elif (len(val) == 4):
            return (val[0])(getValue(type,val[1]), getValue(type, val[2]), \
                getValue(type, val[3]))
        else:
            return (val[0])(getValue(type,val[1]), getValue(type, val[2]), \
                getValue(type, val[3]), getValue(type, val[4]))

    #At this point, we should have been passed a number
    if (isinstance(val, int)):
        return val;

    print('Invalid value '+val+' encountered in getValue\n')

def getStrVal(type, val):
    return str(getValue(type,val))

#Tests which don't depend on the signedness or bit-width of the inputs
generic_tests = {
    'abs': {
        'arg_types': [U, T],
        'function_type': 'ttt',
        'values': [
          [0, 2, [abs,MIN], [abs,MAX]],
          [0, 2,      MIN,       MAX]
        ]
    },
    'abs_diff': {
            'arg_types': [U, T, T],
            'function_type': 'ttt',
            'values': [
               [0, 1, 1, UMAX, UMAX],
               [0, 1, 0, MIN, MAX],
               [0, 0, 1, MAX, MIN]
            ]
    },
    'add_sat': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
           [MAX, MAX        ],
           [MAX, [sub,MAX,1]],
           [ 64, 50         ]
        ]
    },
    'hadd': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [32, 0, 10, MAX, MIN,
                [div,[add,MAX,MIN], 2],
                [div,[add,MIN,MAX], 2]
            ],
            [32, 1, 12, MAX, MIN, MAX, MIN],
            [33, 0,  8, MAX, MIN, MIN, MAX]
        ]
    },
    'clz': {
        'arg_types': [T, T],
        'function_type': 'ttt',
        'values': [
            [ [clz,TYPE,1], [clz,TYPE,64], [clz,TYPE,0], [clz,TYPE, MAX],
              [clz,TYPE,MIN]
            ],
            [           1 ,           64 ,           0 ,            MAX ,
                MIN
            ]
        ]
    },
    'clamp': {
        'arg_types': [T, T, T, T],
        'function_type': 'tss',
        'values': [
            [64, [div, MIN, 2],  1],
            [92,           MIN, 64],
            [ 0, [div, MIN, 2],  0],
            [64,             0,  1]
        ]
    },
    'mad_hi': {
        'arg_types': [T, T, T, T],
        'function_type': 'ttt',
        'values': [
            [ [mad_hi,[div,MAX,2],3,1,TYPE], [mad_hi,MIN,2,2,TYPE], 4, 1,
              [mad_hi,MAX,MAX,MAX,TYPE], [mad_hi,MIN,MIN,MIN,TYPE],
              [mad_hi,MIN,MAX,MAX,TYPE], [mad_hi,MAX,  2,  2,TYPE]
            ],
            [ [div,MAX,2], MIN, 12, MAX, MAX, MIN, MIN, MAX],
            [ 3, 2, 4, 1, MAX, MIN, MAX, 2],
            [ 1, 2, 4, 1, MAX, MIN, MAX, 2]
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
            [0, 0, 0, [mul_hi,MAX,MAX,TYPE], [mul_hi,MIN,MIN,TYPE], 0,
                [mul_hi,MAX,2,TYPE], [div,MIN,2]
            ],
            [0, 92, MAX, MAX, MIN, 92, MAX, MIN],
            [0,  2,   1, MAX, MIN,  1, 2,   MAX]
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
            [1, 8,    1,            2,             1],
            [1, 1,    1,            1,             1],
            [0, 3, SIZE, [add,SIZE,1], [mul,SIZE,10]]
        ]
    },
    'sub_sat': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [1, 25, MAX,   0,   0, MIN,                 MIN],
            [1, 57, MAX, MAX, MIN, MIN,         [div,MIN,2]],
            [0, 32, MIN, MAX, MIN, MAX, [add,[div,MAX,2],1]]
        ]
    },
    'upsample': {
        'arg_types': [B,T,U],
        'function_type': 'ttt',
        'values': [
            [[pow,2,SIZE], [add,[pow,2,SIZE],1], BMAX, 0, MAX,
                [add,[pow,2,SIZE],7]
            ],
            [ 1, 1,  MAX, 0,   0, 1],
            [ 0, 1, UMAX, 0, MAX, 7]
        ]
    }
}

#Any test that conceivably includes a negative number as input/output
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
           [  0, -2, [sub,MAX,63], MIN, MIN         ],
           [ 32, -8, MAX,          MIN, [add,MIN,10]],
           [-32,  6, -63,          -12, -50         ]
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
            [   0,  MIN, MIN, MAX, MIN, -2],
            [  -1,  MIN, MIN, MIN, MIN,  1],
            [ MIN,    2,   1, MIN, MAX, -1],
            [ MIN,    2,  -1, MIN, MAX, -1]
        ]
    },
    'max': {
        'arg_types': [T, T, T],
        'function_type': 'tss',
        'values': [
            [  -1,  1],
            [  -1,  1],
            [ MIN, -1]
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
            [  0,  -1,  -1, -1],
            [ -1, MIN, MIN,  1],
            [MIN,   2,   1, -1]
        ]
    },
    'rhadd': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [-32],
            [-33],
            [-32]
        ]
    },
    'rotate': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [         MIN, MIN, [rot,1,-3,SIZE],             1,
                      MIN,  [pow,2,[sub,SIZE,2]],          MIN],
            [           1,   1,               1,             1,
                        1,                     1,            1],
            [[sub,SIZE,1],  -1,              -3, [mul,SIZE,-1],
             [mul,[add,SIZE,1],-1], [mul,[add,SIZE,2],-1], [sub,SIZE,1]]
        ]
    },
    'sub_sat': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [       MAX ,  81],
            [[sub,MAX,8],   1],
            [       -32 , -80]
        ]
    },
    'upsample': {
        'arg_types': [B,T,U],
        'function_type': 'ttt',
        'values': [
            [   -1, [mul,[pow,2,SIZE],-1]],
            [   -1,                    -1],
            [ UMAX,                     0]
        ]
    }

}

#This list contains all numeric tests which never include negative integers
#that can't go into generic_tests.
unsigned_generic_tests = {
    'mad_sat': {
        'arg_types': [T, T, T, T],
        'function_type': 'ttt',
        'values': [
            [  2, MIN, MAX],
            [MIN, MIN, MIN],
            [  2, MIN, MAX],
            [  2, MIN, MAX]
        ]
    },
    'rotate': {
        'arg_types': [T, T, T],
        'function_type': 'ttt',
        'values': [
            [[div,[add,MAX,1],2], [div,[add,MAX,1],2]],
            [                  1,                   1],
            [       [sub,SIZE,1],        [sub,SIZE,1]]
        ]
    },
}

#Hand-coded tests which are data type specific.
builtins = {
    'int': {
        'mad24':{
            'arg_types': [T, T, T, T],
            'function_type': 'ttt',
            'values': [
                [0,2, 0,520,     1],
                [0,1,-1, 32, 2**30],
                [0,1,-1, 16,     1],
                [0,1,-1,  8,     1]
            ]
        },
        'mul24':{
            'arg_types': [T, T, T],
            'function_type': 'ttt',
            'values': [
                [0,1, 1, 512,     0],
                [0,1,-1,  32, 2**30],
                [0,1,-1,  16,     1]
            ]
        }
    }
}

#### Define helper functions ####

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

#Given a data type and function name, create one set of combined applicable
#test definitions.
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

    #Get the sorted unique set of function names
    return sorted(list(set(fnNames)))

#vecSizes has the layout [in0width, ..., inNwidth] where outType width is
#assumed to match the width of the first input
def gen_kernel(f, fnName, inTypes, outType, vecSizes, typePrefix):
    f.write( \
        'kernel void test_' + typePrefix + str(vecSizes[0]) + '_' + fnName + \
        '_' + inTypes[0]+'(global '+outType+'* out')
    for arg in range(0, len(inTypes)):
        f.write(', global '+inTypes[arg]+'* in'+str(arg))
    f.write('){\n')

    suffix = ';'
    if (vecSizes[0] == 1):
        f.write('  *out = ')
    else:
        f.write('  vstore'+str(vecSizes[0])+'(')
        suffix = ', 0, out)' + suffix

    f.write(fnName+'(')
    suffix = ')' + suffix

    for arg in range(0, len(inTypes)):
        if (arg > 0):
            f.write(', ')
        #if scalar, don't print vload/vstore
        if (vecSizes[arg] == 1):
            f.write('*in'+str(arg))
        else:
            f.write('vload'+str(vecSizes[arg])+'(0, in'+str(arg)+')')

    f.write(suffix+'\n}\n\n')


def getArgType(baseType, argType):
    #If the argType is a string, it's a literal data type... return it
    if (isinstance(argType, str)):
        return argType
    #otherwise it's a list to pick from
    return argType[baseType]

def getArgTypes(baseType, argTypes):
    ret = []
    for argType in argTypes:
        ret.append(getArgType(baseType,argType))
    return ret

#Print a test with all-vector inputs/outputs and/or mixed vector/scalar args
def print_test(f, fnName, argType, functionDef, tests, testIdx, vecSize, tss):
    #If the test allows mixed vector/scalar arguments, handle the case with
    #only vector arguments through a recursive call.
    if (tss):
        print_test(f, fnName, argType, functionDef, tests, testIdx, vecSize, \
            False)

    #The tss && vecSize==1 case is handled in the non-tss case.
    if (tss and vecSize == 1):
        return

    #If we're handling mixed vector/scalar input widths, the kernels have
    #different names than when the vector widths match
    tssStr = 'tss_'
    if (not tss):
        tssStr = ''

    #Write the test header
    f.write('[test]\n' + \
        'name: ' + fnName + ' ' + argType + str(vecSize) + '\n' + \
        'kernel_name: test_'+ tssStr + str(vecSize) + '_' + fnName + '_' + \
        argType + '\n'
    )

    argTypes = getArgTypes(argType, functionDef['arg_types'])
    argCount = len(argTypes)

    #For each argument, write a line containing its type, index, and values
    for arg in range(0, argCount):
        argInOut = ''
        argVal = getStrVal(argType, tests[arg][testIdx])
        if arg == 0:
            argInOut = 'arg_out: '
        else:
            argInOut = 'arg_in: '

        #The output argument and first tss argument are vectors, any that follow
        #are scalar. If !tss, then everything has a matching vector width
        if (arg < 2 or not tss):
            f.write(argInOut + str(arg) + ' buffer ' + argTypes[arg] + \
                '[' + str(vecSize) + '] ' + \
                ' '.join([argVal]*vecSize) + \
                '\n'
            )
        else:
            argInOut = 'arg_in: '
            f.write(argInOut + str(arg) + ' buffer ' + argTypes[arg] + \
                '[1] ' + argVal + '\n'
            )

    #Blank line between tests for formatting reasons
    f.write('\n')

def gen_kernel_1_arg(f, fnName, inType, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, [inType], outType, [vecSize], '')

# 2 argument kernel with input types that match
def gen_kernel_2_arg_same_type(f, fnName, inType, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, [inType, inType], outType, [vecSize, vecSize], '')

# 2 argument kernel with 1 vector and one scalar input argument
def gen_kernel_2_arg_mixed_size(f, fnName, inType, outType):
    for vecSize in VEC_WIDTHS:
        gen_kernel(f, fnName, [inType, inType], outType, [vecSize, 1], 'tss_')

# 2 argument kernel with 1 vector and one scalar input argument with multiple
#   input data types
def gen_kernel_2_arg_mixed_sign(f, fnName, inType1, inType2, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, [inType1, inType2], outType, [vecSize, vecSize], \
            '')

# 3-argument built-in functions

def gen_kernel_3_arg_same_type(f, fnName, inType, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, [inType, inType, inType], outType, \
            [vecSize, vecSize, vecSize], '')

def gen_kernel_3_arg_mixed_size_vector(f, fnName, inType, outType, vecSize):
    f.write( \
        'kernel void test_tss_' + vecSize + '_' + fnName + '_' + inType + \
        '(global ' + outType + '* out, global ' + inType + '* in1, global ' + \
        inType+'* in2, global '+inType+'* in3){\n' + \
        '  vstore' + vecSize + '(' + fnName + '(vload' + vecSize + \
        '(0, in1), *in2, *in3), 0, out);\n' + \
        '}\n\n'
    )

def gen_kernel_3_arg_mixed_size(f, fnName, inType, outType):
    for vecSize in VEC_WIDTHS:
        gen_kernel(f, fnName, [inType, inType, inType], outType, \
            [vecSize, 1, 1], 'tss_')

def generate_kernels(f, dataType, fnName, fnDef):
    argTypes = getArgTypes(dataType,fnDef['arg_types'])

    #For len(argTypes), remember that this includes the output arg
    if (len(argTypes) == 2):
        gen_kernel_1_arg(f, fnName, argTypes[1], argTypes[0])
        return

    if (len(argTypes) == 3 and not fnName is 'upsample'):
        gen_kernel_2_arg_same_type(f, fnName, argTypes[1], argTypes[0])
        if (fnDef['function_type'] is 'tss'):
            gen_kernel_2_arg_mixed_size(f, fnName, argTypes[1], argTypes[0])
        return

    if (len(argTypes) == 4):
        gen_kernel_3_arg_same_type(f, fnName, argTypes[1], argTypes[0])
        if (fnDef['function_type'] is 'tss'):
            gen_kernel_3_arg_mixed_size(f, fnName, argTypes[1], argTypes[0])
        return

    if (fnName is 'upsample'):
        gen_kernel_2_arg_mixed_sign(f, fnName, argTypes[1], argTypes[2], \
            argTypes[0])
        return

#### Main logic start ####

def main():
    #Create the output directory if required
    dirName = os.path.join( "cl", "builtin", "int")
    if not os.path.exists(dirName):
        os.makedirs(dirName)

    #Loop over all data types being tested. Create one output file per data type
    for dataType in DATA_TYPES:
        functions = getFnNames() #List of all built-in functions
        for fnName in functions:
            if (fnName is 'upsample' and (dataType is 'long' \
                or dataType is 'ulong')):
                continue
            #Merge all of the generic/signed/unsigned/custom test definitions
            functionDef = mergedTestDefinition(dataType, fnName)

            #Check if the function actually exists for this data type
            if (not functionDef.keys()):
                continue

            clcVersionMin = CLC_VERSION_MIN[fnName]

            fileName = 'builtin-' + dataType + '-' + fnName + '-' + \
                str(float(clcVersionMin)/10)+'.generated.cl'

            fileName = os.path.join(dirName, fileName)

            f = open(fileName, 'w')
            print(fileName)
            #Write the file header
            f.write( \
                '/*!\n' + \
                '[config]\n' + \
                'name: Test '+dataType+' '+fnName+' built-in on CL 1.1\n'+ \
                'clc_version_min: '+str(clcVersionMin)+'\n' + \
                'dimensions: 1\n' + \
                'global_size: 1 0 0\n\n'
            )

            #Write all tests for the built-in function
            tests = functionDef['values']
            argCount = len(functionDef['arg_types'])
            fnType = functionDef['function_type']

            outputValues = tests[0]
            numTests = len(outputValues)

            #Handle all available scalar/vector widths
            sizes = sorted(VEC_WIDTHS)
            sizes.insert(0,1) #Add 1-wide scalar to the vector widths
            for vecSize in sizes:
                for testIdx in range(0, numTests):
                    print_test(f, fnName, dataType, functionDef, tests, \
                        testIdx, vecSize, (fnType is 'tss'))

            #Terminate the header section
            f.write('!*/\n\n')

            #Generate the actual kernels
            generate_kernels(f, dataType, fnName, functionDef)

        #Hopefully this next part is obvious :)
        f.close()
main()
