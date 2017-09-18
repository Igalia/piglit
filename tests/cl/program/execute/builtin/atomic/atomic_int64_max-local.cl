/*!
[config]
name: atom_int64_max local
clc_version_min: 10
require_device_extensions: cl_khr_int64_extended_atomics

[test]
name: simple long
kernel_name: simple_long
dimensions: 1
global_size: 1 0 0
local_size: 1 0 0
arg_out: 0 buffer long[2] -1 2
arg_in:  1 buffer long[1] NULL
arg_in:  2 long           -1
arg_in:  3 long            2

[test]
name: simple ulong
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer ulong[2] 2 3
arg_in:  1 buffer ulong[1] NULL
arg_in:  2 ulong           2
arg_in:  3 ulong           3

[test]
name: simple ulong 2
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer ulong[2] 3 4294967295
arg_in:  1 buffer ulong[1] NULL
arg_in:  2 ulong           3
arg_in:  3 ulong           0xffffffff


[test]
name: threads long
kernel_name: threads_long
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer long[1] 7
arg_in:  1 buffer long[1] NULL

[test]
name: threads ulong
kernel_name: threads_ulong
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer ulong[1] 7
arg_in:  1 buffer ulong[1] NULL

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *out, local TYPE *mem, TYPE initial, TYPE other) { \
	*mem = initial; \
	TYPE a = atom_max(mem, other); \
	out[0] = a; \
	out[1] = *mem; \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out, local TYPE *mem) { \
	*mem = 0; \
	barrier(CLK_LOCAL_MEM_FENCE); \
	atom_max(mem, get_global_id(0)); \
	barrier(CLK_LOCAL_MEM_FENCE); \
	*out = *mem; \
}

SIMPLE_TEST(long)
SIMPLE_TEST(ulong)

THREADS_TEST(long)
THREADS_TEST(ulong)
