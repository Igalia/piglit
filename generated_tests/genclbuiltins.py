from __future__ import print_function, division
import os

import six

__all__ = ['gen', 'DATA_SIZES', 'MAX_VALUES', 'MAX', 'MIN', 'BMIN', 'BMAX',
           'SMIN', 'SMAX', 'UMIN', 'UMAX', 'TYPE', 'T', 'U', 'B']

_NUMERIC_TYPES = tuple(list(six.integer_types) + [float])


DATA_SIZES = {
    'char': 8,
    'uchar': 8,
    'short': 16,
    'ushort': 16,
    'int': 32,
    'uint': 32,
    'long': 64,
    'ulong': 64
}

# By default, just test what is part of the CL1.1 spec, leave vec3 for later
# VEC_WIDTHS = [2, 3, 4, 8, 16]
VEC_WIDTHS = [2, 4, 8, 16]

# ALL_WIDTHS = [1, 2, 3, 4, 8, 16]
ALL_WIDTHS = [1, 2, 4, 8, 16]

MIN_VALUES = {
    'char': -128,
    'uchar': 0,
    'short': -32768,
    'ushort': 0,
    'int': -2147483648,
    'uint': 0,
    'long': -9223372036854775808,
    'ulong': 0
}

MAX_VALUES = {
    'char': 127,
    'uchar': 255,
    'short': 32767,
    'ushort': 65535,
    'int': 2147483647,
    'uint': 4294967295,
    'long': 9223372036854775807,
    'ulong': 18446744073709551615
}

# Define placeholders to reduce magic number usage
MAX = 'MAX_VAL'
MIN = 'MIN_VAL'
BMIN = 'min_for_larger_type'
BMAX = 'max_for_larger_type'
SMIN = 'signed_min_for_type'
SMAX = 'signed_max_for_type'
UMIN = 'unsigned_min_for_type'
UMAX = 'unsigned_max_for_type'
TYPE = 'TYPE'
SIZE = 'SIZE'
TRUE = 'true_value_for_type' #1 for scalar, -1 for vector
NEGNAN = 'Negative NAN as a string, because float("-nan") just produces nan'

# Identity type list
T = {
    'char': 'char',
    'uchar': 'uchar',
    'short': 'short',
    'ushort': 'ushort',
    'int': 'int',
    'uint': 'uint',
    'long': 'long',
    'ulong': 'ulong'
}
# Signed type for each type
SIGNED = {
    'char': 'char',
    'uchar': 'char',
    'short': 'short',
    'ushort': 'short',
    'int': 'int',
    'uint': 'int',
    'long': 'long',
    'ulong': 'long'
}
# Unsigned type for each source type
U = {
    'char': 'uchar',
    'uchar': 'uchar',
    'short': 'ushort',
    'ushort': 'ushort',
    'int': 'uint',
    'uint': 'uint',
    'long': 'ulong',
    'ulong': 'ulong'
}
# Next larger type with same signedness
B = {
    'char': 'short',
    'uchar': 'ushort',
    'short': 'int',
    'ushort': 'uint',
    'int': 'long',
    'uint': 'ulong',
}


# vecSizes has the layout [in0width, ..., inNwidth] where outType width is
# assumed to match the width of the first input
def gen_kernel(f, fnName, inTypes, outType, vecSizes, typePrefix):
    f.write('kernel void test_' + typePrefix + str(vecSizes[0]) + '_' + fnName
            + '_' + inTypes[0]+'(global '+outType+'* out')
    for arg in range(0, len(inTypes)):
        f.write(', global '+inTypes[arg]+'* in'+str(arg))
    f.write('){\n')

    suffix = ';'
    if (vecSizes[0] == 1):
        f.write('  out[get_global_id(0)] = ')
    else:
        f.write('  vstore'+str(vecSizes[0])+'(')
        suffix = ', get_global_id(0), out)' + suffix

    f.write(fnName+'(')
    suffix = ')' + suffix

    for arg in range(0, len(inTypes)):
        if (arg > 0):
            f.write(', ')
        # if scalar, don't print vload/vstore
        if (vecSizes[arg] == 1):
            f.write('in'+str(arg)+'[get_global_id(0)]')
        else:
            f.write('vload'+str(vecSizes[arg])+'(get_global_id(0), in'+str(arg)+')')

    f.write(suffix+'\n}\n\n')



def gen_kernel_1_arg(f, fnName, inType, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, [inType], outType, [vecSize], '')


#  2 argument kernel with input types that match their vector size
def gen_kernel_2_arg_same_size(f, fnName, inTypes, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, inTypes, outType, [vecSize, vecSize],
                   '')


#  2 argument kernel with 1 vector and one scalar input argument
def gen_kernel_2_arg_mixed_size(f, fnName, inTypes, outType):
    for vecSize in VEC_WIDTHS:
        gen_kernel(f, fnName, inTypes, outType, [vecSize, 1], 'tss_')


#  2 argument kernel with 1 vector and one scalar input argument with multiple
#    input data types
def gen_kernel_2_arg_mixed_sign(f, fnName, inTypes, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, inTypes, outType, [vecSize, vecSize],
                   '')


#  3-argument built-in functions


def gen_kernel_3_arg_same_type(f, fnName, inTypes, outType):
    for vecSize in ALL_WIDTHS:
        gen_kernel(f, fnName, inTypes, outType,
                   [vecSize, vecSize, vecSize], ''
        )

def gen_kernel_3_arg_mixed_size_tss(f, fnName, inTypes, outType):
    for vecSize in VEC_WIDTHS:
        gen_kernel(f, fnName, inTypes, outType,
                   [vecSize, 1, 1], 'tss_')

def gen_kernel_3_arg_mixed_size_tts(f, fnName, inTypes, outType):
    for vecSize in VEC_WIDTHS:
        gen_kernel(f, fnName, inTypes, outType,
                   [vecSize, vecSize, 1], 'tts_')


def generate_kernels(f, dataType, fnName, fnDef):
    argTypes = getArgTypes(dataType, fnDef['arg_types'])

    # For len(argTypes), remember that this includes the output arg
    if (len(argTypes) == 2):
        gen_kernel_1_arg(f, fnName, argTypes[1], argTypes[0])
        return

    if (len(argTypes) == 3 and not fnName is 'upsample'):
        gen_kernel_2_arg_same_size(f, fnName,
                                [argTypes[1], argTypes[2]], argTypes[0])
        if (fnDef['function_type'] is 'tss'):
            gen_kernel_2_arg_mixed_size(f, fnName,
                                [argTypes[1], argTypes[2]], argTypes[0])
        return

    if (len(argTypes) == 4):
        gen_kernel_3_arg_same_type(f, fnName,
                   [argTypes[1], argTypes[2], argTypes[3]], argTypes[0])
        if (fnDef['function_type'] is 'tss'):
            gen_kernel_3_arg_mixed_size_tss(f, fnName,
                   [argTypes[1], argTypes[2], argTypes[3]], argTypes[0])
        if (fnDef['function_type'] is 'tts'):
            gen_kernel_3_arg_mixed_size_tts(f, fnName,
                   [argTypes[1], argTypes[2], argTypes[3]], argTypes[0])
        return

    if (fnName is 'upsample'):
        gen_kernel_2_arg_mixed_sign(f, fnName,
                                    [argTypes[1], argTypes[2]],
                                    argTypes[0])
        return

def getValue(type, val, isVector):
    # Check if val is a str, list, or value
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
        elif (val == TRUE):
            if (isVector):
                return -1
            else:
                return 1
        elif (val == NEGNAN):
            return '-nan' #cl-program-tester translates this for us
        else:
            print('Unknown string value: ' + val + '\n')
    elif (isinstance(val, list)):
        # The list should be of the format: [op, arg1, ... argN] where op is a
        # Fn ref and arg[1-n] are either MIN/MAX or numbers (They could be
        # nested lists). The exception for arg1 is TYPE, which means to
        # substitute the data type

        # Evaluate the value of the requested function and arguments
        # TODO: Change to varargs calls after unshifting the first list element
        if (callable(val[0])):
            if (len(val) == 2):
                return (val[0])(getValue(type, val[1], isVector))
            elif (len(val) == 3):
                return (val[0])(getValue(type, val[1], isVector), getValue(type, val[2], isVector))
            elif (len(val) == 4):
                return (val[0])(getValue(type, val[1], isVector), getValue(type, val[2], isVector),
                            getValue(type, val[3], isVector))
            else:
                return (val[0])(getValue(type, val[1], isVector), getValue(type, val[2], isVector),
                            getValue(type, val[3], isVector), getValue(type, val[4], isVector))
        else:
             return map(lambda x: getValue(type, x, isVector), val);

    # At this point, we should have been passed a number
    if (isinstance(val, _NUMERIC_TYPES)):
        return val

    print('Invalid value '+repr(val)+' encountered in getValue\n')



def getStrVal(type, val, isVector):
    return " ".join(map(str, getValue(type, val, isVector)))


def getArgType(baseType, argType):
    # If the argType is a string, it's a literal data type... return it
    if (isinstance(argType, str)):
        return argType
    # otherwise it's a list to pick from
    return argType[baseType]


def getArgTypes(baseType, argTypes):
    ret = []
    for argType in argTypes:
        ret.append(getArgType(baseType, argType))
    return ret

def isFloatType(t):
    return t not in U

# Print a test with all-vector inputs/outputs and/or mixed vector/scalar args
def print_test(f, fnName, argType, functionDef, tests, numTests, vecSize, tss):
    # If the test allows mixed vector/scalar arguments, handle the case with
    # only vector arguments through a recursive call.
    if (tss):
        print_test(f, fnName, argType, functionDef, tests, numTests, vecSize,
                   False)

    # The tss && vecSize==1 case is handled in the non-tss case.
    if (tss and vecSize == 1):
        return

    # If we're handling mixed vector/scalar input widths, the kernels have
    # different names than when the vector widths match
    tssStr = 'tss_'
    if (not tss):
        tssStr = ''

    argTypes = getArgTypes(argType, functionDef['arg_types'])
    argCount = len(argTypes)
    tolerance = functionDef['tolerance'] if 'tolerance' in functionDef else 0

    # Write the test header
    f.write('[test]\n' + 'name: ' + tssStr + fnName + ' ' + argType
            + str(vecSize) + '\n' + 'kernel_name: test_' + tssStr + str(vecSize)
            + '_' + fnName + '_' + argType + '\n' + 'global_size: '
            + str(numTests) + ' 0 0\n\n'
    )

    # For each argument, write a line containing its type, index, and values
    for arg in range(0, argCount):
        argInOut = ''
        argVal = getStrVal(argType, tests[arg], (vecSize > 1))
        if arg == 0:
            argInOut = 'arg_out: '
        else:
            argInOut = 'arg_in: '

        # The output argument and first tss argument are vectors, any that
        # follow are scalar. If !tss, then everything has a matching vector
        # width
        if (arg < 2 or not tss):
            f.write(argInOut + str(arg) + ' buffer ' + argTypes[arg] +
                    '[' + str(numTests * vecSize) + '] ' +
                    ''.join(map(lambda x: (x + ' ') * vecSize, argVal.split()))
            )
            if arg == 0:
                f.write(' tolerance {0} '.format(tolerance))
                # Use ulp tolerance for float types
                if isFloatType(argTypes[arg]):
                    f.write('ulp')
            f.write('\n')
        else:
            argInOut = 'arg_in: '
            f.write(argInOut + str(arg) + ' buffer ' + argTypes[arg] + '[' +
                    str(numTests) + '] ' + argVal + '\n'
            )

    # Blank line between tests for formatting reasons
    f.write('\n')


def gen(types, minVersions, functions, testDefs, dirName):
    # Create the output directory if required
    if not os.path.exists(dirName):
        try:
            os.makedirs(dirName)
        except OSError as e:
            if e.errno == 17:  # file exists
                pass
            raise

    # Loop over all data types being tested. Create one output file per data
    # type
    for dataType in types:
        for fnName in functions:
            # Merge all of the generic/signed/unsigned/custom test definitions
            if (dataType, fnName) not in testDefs:
                continue
            functionDef = testDefs[(dataType, fnName)]

            # Check if the function actually exists for this data type
            if (not functionDef.keys()):
                continue

            clcVersionMin = minVersions[fnName]

            fileName = 'builtin-' + dataType + '-' + fnName + '-' + \
                str(float(clcVersionMin)/10)+'.generated.cl'

            fileName = os.path.join(dirName, fileName)

            f = open(fileName, 'w')
            print(fileName)
            # Write the file header
            f.write('/*!\n' +
                    '[config]\n' +
                    'name: Test '+dataType+' '+fnName+' built-in on CL 1.1\n' +
                    'clc_version_min: '+str(clcVersionMin)+'\n' +
                    'dimensions: 1\n'
            )
            if (dataType == 'double'):
                f.write('require_device_extensions: cl_khr_fp64\n')

            # Blank line  to provide separation between config header and tests
            f.write('\n')

            # Write all tests for the built-in function
            tests = functionDef['values']
            argCount = len(functionDef['arg_types'])
            fnType = functionDef['function_type']

            outputValues = tests[0]
            numTests = len(outputValues)

            # Handle all available scalar/vector widths
            sizes = sorted(VEC_WIDTHS)
            sizes.insert(0, 1)  # Add 1-wide scalar to the vector widths
            for vecSize in sizes:
                print_test(f, fnName, dataType, functionDef, tests,
                           numTests, vecSize, (fnType is 'tss'))

            # Terminate the header section
            f.write('!*/\n\n')

            if (dataType == 'double'):
                f.write('#pragma OPENCL EXTENSION cl_khr_fp64 : enable\n\n')

            # Generate the actual kernels
            generate_kernels(f, dataType, fnName, functionDef)

        f.close()
