/*!
[config]
name: atom_int64_or global, with usage of return variable
clc_version_min: 10
require_device_extensions: cl_khr_int64_extended_atomics

[test]
name: simple long
kernel_name: simple_long
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer long[2] -3 5
arg_in:  0 buffer long[2]  5 0
arg_in:  1        long    -4

[test]
name: simple ulong
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer ulong[2] 14 6
arg_in:  0 buffer ulong[2]  6 0
arg_in:  1        ulong    10

[test]
name: threads long
kernel_name: threads_long
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer long[18] 7 0 7 7 3 2 6 4 3 0 6 2 7 2 14 8 15 8
arg_in:  0 buffer long[18] 0 0 7 0 2 0 4 0 0 0 2 0 2 0  8 0  8 0

[test]
name: threads ulong
kernel_name: threads_ulong
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer long[18] 7 0 7 7 3 2 6 4 3 0 6 2 7 2 14 8 15 8
arg_in:  0 buffer long[18] 0 0 7 0 2 0 4 0 0 0 2 0 2 0  8 0  8 0

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *mem, TYPE value) { \
  mem[1] = atom_or(mem, value); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *mem) { \
  TYPE mul = mem[1]; \
  TYPE id = get_global_id(0); \
  TYPE ret = atom_or(mem, id); \
  TYPE ret2 = atom_or(&mem[(id+1)*2], id+ret*mul); \
  mem[(id+1)*2+1] = ret2; \
}

SIMPLE_TEST(long)
SIMPLE_TEST(ulong)

THREADS_TEST(long)
THREADS_TEST(ulong)
