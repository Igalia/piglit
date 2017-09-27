/*!
[config]
name: atom_int32_add global, no usage of return variable
clc_version_min: 10
require_device_extensions: cl_khr_global_int32_base_atomics

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[1] -4
arg_in:  0 buffer int[1] -5

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer uint[1] 1
arg_in:  0 buffer uint[1] 0

[test]
name: threads int
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[1] 28
arg_in:  0 buffer int[1] 0

[test]
name: threads uint
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[1] 28
arg_in:  0 buffer uint[1] 0

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *mem) { \
  atom_add(mem, 1); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *mem) { \
  TYPE id = get_global_id(0); \
  atom_add(mem, id); \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)

THREADS_TEST(int)
THREADS_TEST(uint)
