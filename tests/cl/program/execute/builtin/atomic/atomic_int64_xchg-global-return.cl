/*!
[config]
name: atom_int64_xchg global
clc_version_min: 10
require_device_extensions: cl_khr_int64_base_atomics

[test]
name: simple long
kernel_name: simple_long
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer long[2] -4 1
arg_in:  0 buffer long[2]  1 0
arg_in:  1 long           -4

[test]
name: simple ulong
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer long[2] 4 2
arg_in:  0 buffer long[2] 2 0
arg_in:  1 long           4

[test]
name: threads long
kernel_name: threads_long
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer long[1] 7
arg_in:  0 buffer long[1] -1
arg_out: 1 buffer long[8] -1 0 1 2 3 4 5 6

[test]
name: threads ulong
kernel_name: threads_ulong
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer ulong[1] 7
arg_in:  0 buffer ulong[1] 9
arg_out: 1 buffer long[8] 9 0 1 2 3 4 5 6

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *out, TYPE value) { \
	out[1] = atom_xchg(out, value); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out, global TYPE *old) { \
	long i; \
	TYPE id = get_global_id(0); \
	barrier(CLK_GLOBAL_MEM_FENCE); \
	for(i = 0; i < get_global_size(0); i++){ \
		if (i == id){ \
			old[i] = atom_xchg(out, (TYPE)id); \
		} \
		barrier(CLK_GLOBAL_MEM_FENCE); \
	} \
}

SIMPLE_TEST(long)
SIMPLE_TEST(ulong)

THREADS_TEST(long)
THREADS_TEST(ulong)
