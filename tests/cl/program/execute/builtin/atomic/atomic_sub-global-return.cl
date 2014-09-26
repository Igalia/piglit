/*!
[config]
name: atomic_sub global, with usage of return variable
clc_version_min: 11

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[2] -6 -4
arg_in:  0 buffer int[2] -4 0

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer uint[2] 1 3
arg_in:  0 buffer uint[2] 3 0

[test]
name: threads int
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[18] 0  0 1 1 1 2 1 3 1 4 1 5 1 6 1 7 1 8
arg_in:  0 buffer int[18] 28 0 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0

[test]
name: threads uint
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[18] 0  0 1 1 1 2 1 3 1 4 1 5 1 6 1 7 1 8
arg_in:  0 buffer uint[18] 28 0 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *mem) { \
  mem[1] = atomic_sub(mem, 2); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *mem) { \
  TYPE mul = mem[1]; \
  TYPE id = get_global_id(0); \
  TYPE ret = atomic_sub(mem, id); \
  TYPE ret2 = atomic_sub(&mem[(id+1)*2], id+ret*mul); \
  mem[(id+1)*2+1] = ret2; \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)

THREADS_TEST(int)
THREADS_TEST(uint)
