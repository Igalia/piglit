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
arg_out: 0 buffer long[1] -4
arg_in:  0 buffer long[1] 1
arg_in:  1 long           -4

[test]
name: simple ulong
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer long[1] 4
arg_in:  0 buffer long[1] 2
arg_in:  1 long           4

[test]
name: threads long
kernel_name: threads_long
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer long[1] 7
arg_in:  0 buffer long[1] 0

[test]
name: threads ulong
kernel_name: threads_ulong
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer ulong[1] 7
arg_in:  0 buffer ulong[1] 0

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *out, TYPE value) { \
	atom_xchg(out, value); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out) { \
	long i; \
	TYPE id = get_global_id(0); \
	barrier(CLK_GLOBAL_MEM_FENCE); \
	for(i = 0; i < get_global_size(0); i++){ \
		if (i == id){ \
			atom_xchg(out, (TYPE)id); \
		} \
		barrier(CLK_GLOBAL_MEM_FENCE); \
	} \
}

SIMPLE_TEST(long)
SIMPLE_TEST(ulong)

THREADS_TEST(long)
THREADS_TEST(ulong)
