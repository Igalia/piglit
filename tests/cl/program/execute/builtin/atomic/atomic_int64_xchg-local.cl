/*!
[config]
name: atom_int64_xchg local
clc_version_min: 10
require_device_extensions: cl_khr_int64_base_atomics

[test]
name: simple long
kernel_name: simple_long
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer long[2] -4 5
arg_in:  1 buffer long[1] NULL
arg_in:  2 long           -4
arg_in:  3 long           5

[test]
name: simple ulong
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer ulong[2] 4 5
arg_in:  1 buffer ulong[1] NULL
arg_in:  2 ulong           4
arg_in:  3 ulong           5

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
kernel void simple_##TYPE(global TYPE *out, local TYPE *mem, TYPE initial, TYPE value) { \
	*mem = initial; \
	TYPE a = atom_xchg(mem, value); \
	out[0] = a; \
	out[1] = *mem; \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out, local TYPE *mem) { \
	long i; \
	*mem = 0; \
	barrier(CLK_LOCAL_MEM_FENCE); \
	TYPE id = get_local_id(0); \
	for(i = 0; i < get_local_size(0); i++){ \
		if (i == id){ \
			atom_xchg(mem, (TYPE)id); \
		} \
		barrier(CLK_LOCAL_MEM_FENCE); \
	} \
	*out = *mem; \
}

SIMPLE_TEST(long)
SIMPLE_TEST(ulong)

THREADS_TEST(long)
THREADS_TEST(ulong)
