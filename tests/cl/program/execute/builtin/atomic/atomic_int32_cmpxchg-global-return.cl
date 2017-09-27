/*!
[config]
name: atom_int32_cmpxchg global return
clc_version_min: 10
require_device_extensions: cl_khr_global_int32_base_atomics

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[2]  5 -4
arg_in:  0 buffer int[2] -4 -4
arg_in:  1 buffer int[2] -4  3
arg_in:  2 buffer int[2]  5  5
arg_out: 3 buffer int[2] -4 -4

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer uint[2] 5 4
arg_in:  0 buffer uint[2] 4 4
arg_in:  1 buffer uint[2] 4 3
arg_in:  2 buffer uint[2] 5 5
arg_out: 3 buffer uint[2] 4 4

[test]
name: threads int
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[1] 8
arg_in:  0 buffer int[1] 0
arg_out: 1 buffer int[8] 0 1 2 3 4 5 6 7

[test]
name: threads uint
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[1] 8
arg_in:  0 buffer uint[1] 0
arg_out: 1 buffer uint[8] 0 1 2 3 4 5 6 7

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *initial, global TYPE *compare, global TYPE *value, global TYPE *old) { \
	old[0] = atom_cmpxchg(initial, compare[0], value[0]); \
	old[1] = atom_cmpxchg(initial+1, compare[1], value[1]); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out, global TYPE *old) { \
	int i; \
	barrier(CLK_GLOBAL_MEM_FENCE); \
	TYPE id = get_global_id(0); \
	for(i = 0; i < get_global_size(0); i++){ \
		TYPE old_val = atom_cmpxchg(out, id, id+1); \
		if (old_val == id) /* success */ \
			old[id] = old_val; \
		barrier(CLK_GLOBAL_MEM_FENCE); \
	} \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)

THREADS_TEST(int)
THREADS_TEST(uint)
