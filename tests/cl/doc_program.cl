/*!
#^^ Configuration start

# This is an OpenCL C file for program-tester tests
# This type of test should be used for testing program building and/or
# kernel execution.
# Empty template is in template_program.cl

# This is a comment


# Configuration #

[config]

name: Add and subtract        # Name of the test
clc_version_min: 10           # Minimum requirec OpenCL C version
clc_version_max: 12           # Maximum required OpenCL C version
#platform_regex: .*Gallium.*  # Only run on platforms that match this POSIX regex
#device_regex: .*RV300.*      # Only run on devices that match this POSIX regex
#require_platform_extensions: ext1 ext2  # Space-delimited required platform extensions
#require_device_extensions:   ext1 ext2  # Space-delimited required device extensions

build_options: -D DEF        # Build options for the program
#expect_build_fail: true     # Expect that build will fail

kernel_name: add             # Default kernel to run
expect_test_fail: true       # Expect that tests will fail (arguments won't check out)

dimensions: 3                # Number of dimensions for ND kernel (default: 1)
global_size: 10 10 10        # Global work size for ND kernel (default: 1 0 0)
local_size:   2  2  2        # Local work size for ND kernel (default: NULL)


# Execution tests #

[test]
name: Add               # Execution test name
kernel_name: add        # Override kernel_name from [config]
expect_test_fail: false # Override expect_test_fail from [config]
dimensions: 1           # Override dimensions from [config]
global_size: 15 15 15   # Override global_size from [config]
local_size:   1  1  1   # Override global_size from [config]

arg_in: 0 buffer int[15] 0   1   2   3   4 \    # Buffer argument with data to initialize it.
                         5   6   7   8   9 \    # \ is used for a multiline value.
                       0XA 0XB 0XC 0XD 0XE      # Hex values are supported.
arg_in: 2 float 1.0                  # Float argument
arg_out: 1 buffer uint[15]  1  2  3  4  5 \  # Buffer argument with expected data.
                            6  7  8  9 10 \  # Same argument can have different types as long
                           11 12 13 14 15    # as the size of the data is correct. For example
                                             # arg_in can be of type int[4] and arg_out of type uchar[16]

[test]
name: Subtract          # Execution test name
kernel_name: sub        # Override kernel_name from [config]
expect_test_fail: false # Override expect_test_fail from [config]
dimensions: 1           # Override dimensions from [config]
global_size: 6 6 6      # Override global_size from [config]
local_size:  2 2 2      # Override global_size from [config]

arg_in: 0 buffer float2[6] repeat 5 4  # Buffer argument with repeated data to initialize it (5 4 5 4 5 4)
arg_in: 1 buffer float2[6] random      # Buffer argument with random data to initialize it
                                       # (not needed here, but provided as example)
arg_in: 2 float2 1 1                   # Int argument
arg_out: 1 buffer float2[6] repeat 4.1 3.1 \ # Buffer argument with repeated expected data (4.1 3.1 4.1 3.1 4.1 3.1)
                            tolerance 0.1    # Tolerate result data with +-0.1 offset

#ˇˇ Configuration end
!*/

/* 
 * Every test is passed an OpenCL C version.
 * For example version 1.2 is passed as 120 in
 * __OPENCL_C_VERSION__
 */
#if __OPENCL_C_VERSION__ >= 120
const double d = 15;
#endif

kernel void add(global int* x, global int* y, float z) {
	int i = get_global_id(0);
	y[i] = x[i]+z;
}

kernel void sub(global float2* x, global float2* y, float2 z) {
	int i = get_global_id(0);
	y[i] = x[i]-z;
}
