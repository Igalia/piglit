/*!
[config]
name: atom_int64_min global, no usage of return variable
clc_version_min: 10
require_device_extensions: cl_khr_int64_extended_atomics

[test]
name: simple long
kernel_name: simple_long
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer long[1] -5
arg_in:  0 buffer long[1] -5

[test]
name: simple ulong
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer ulong[1] 1
arg_in:  0 buffer ulong[1] 2

[test]
name: threads long
kernel_name: threads_long
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer long[1] 0
arg_in:  0 buffer long[1] 7

[test]
name: threads ulong
kernel_name: threads_ulong
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer ulong[1] 0
arg_in:  0 buffer ulong[1] 7

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *mem) { \
  atom_min(mem, 1); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *mem) { \
  TYPE id = get_global_id(0); \
  atom_min(mem, id); \
}

SIMPLE_TEST(long)
SIMPLE_TEST(ulong)

THREADS_TEST(long)
THREADS_TEST(ulong)
