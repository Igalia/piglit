/*!
[config]
name: atom_int64_cmpxchg global return
clc_version_min: 10
require_device_extensions: cl_khr_int64_base_atomics

[test]
name: simple long
kernel_name: simple_long
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer long[2]  5 -4
arg_in:  0 buffer long[2] -4 -4
arg_in:  1 buffer long[2] -4  3
arg_in:  2 buffer long[2]  5  5
arg_out: 3 buffer long[2] -4 -4

[test]
name: simple ulong
kernel_name: simple_ulong
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer ulong[2] 5 4
arg_in:  0 buffer ulong[2] 4 4
arg_in:  1 buffer ulong[2] 4 3
arg_in:  2 buffer ulong[2] 5 5
arg_out: 3 buffer ulong[2] 4 4

[test]
name: threads long
kernel_name: threads_long
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer long[1] 8
arg_in:  0 buffer long[1] 0
arg_out: 1 buffer long[8] 0 1 2 3 4 5 6 7

[test]
name: threads ulong
kernel_name: threads_ulong
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer ulong[1] 8
arg_in:  0 buffer ulong[1] 0
arg_out: 1 buffer ulong[8] 0 1 2 3 4 5 6 7

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *initial, global TYPE *compare, global TYPE *value, global TYPE *old) { \
	old[0] = atom_cmpxchg(initial, compare[0], value[0]); \
	old[1] = atom_cmpxchg(initial+1, compare[1], value[1]); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out, global TYPE *old) { \
	long i; \
	barrier(CLK_GLOBAL_MEM_FENCE); \
	TYPE id = get_global_id(0); \
	for(i = 0; i < get_global_size(0); i++){ \
		TYPE old_val = atom_cmpxchg(out, id, id+1); \
		if (old_val == id) /* success */ \
			old[id] = old_val; \
		barrier(CLK_GLOBAL_MEM_FENCE); \
	} \
}

SIMPLE_TEST(long)
SIMPLE_TEST(ulong)

THREADS_TEST(long)
THREADS_TEST(ulong)
