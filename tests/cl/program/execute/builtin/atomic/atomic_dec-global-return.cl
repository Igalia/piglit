/*!
[config]
name: atomic_dec global, with usage of return variable
clc_version_min: 11

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[2] -5 -4
arg_in:  0 buffer int[2] -4 0

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer uint[2] 1 2
arg_in:  0 buffer uint[2] 2 0

[test]
name: threads int
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[18] -9 0 -1 0 1 2 2 3 3 4 4 5 5 6 6 7 7 8
arg_in:  0 buffer int[18] -1 0  0 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0

[test]
name: threads uint
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[18] 0 0 0 1 1 2 2 3 3 4 4 5 5 6 6 7 7 8
arg_in:  0 buffer uint[18] 8 0 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *mem) { \
  mem[1] = atomic_dec(mem); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *mem) { \
  TYPE mul = mem[1]; \
  TYPE id = get_global_id(0); \
  TYPE ret = atomic_dec(mem); \
  TYPE ret2 = atomic_dec(&mem[(id+1)*2]); \
  mem[(id+1)*2+1] = ret2; \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)

THREADS_TEST(int)
THREADS_TEST(uint)
