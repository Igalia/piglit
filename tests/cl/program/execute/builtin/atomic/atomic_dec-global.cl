/*!
[config]
name: atomic_dec global, no usage of return variable
clc_version_min: 11

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[1] -5
arg_in:  0 buffer int[1] -4

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer uint[1] 1
arg_in:  0 buffer uint[1] 2

[test]
name: threads int
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[1] -8
arg_in:  0 buffer int[1]  0

[test]
name: threads uint
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[1] 1
arg_in:  0 buffer uint[1] 9

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *mem) { \
  atomic_dec(mem); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *mem) { \
  atomic_dec(mem); \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)

THREADS_TEST(int)
THREADS_TEST(uint)
