/*!
[config]
name: atomic_cmpxchg local
clc_version_min: 11

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[4] -4 5 -4 -4
arg_in:  1 buffer int[2] NULL
arg_in:  2 buffer int[2] -4 -4
arg_in:  3 buffer int[2] -4 3
arg_in:  4 buffer int[2]  5 5

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer uint[4] 4 5 4 4
arg_in:  1 buffer uint[2] NULL
arg_in:  2 buffer uint[2] 4 4
arg_in:  3 buffer uint[2] 4 3
arg_in:  4 buffer uint[2] 5 5

[test]
name: threads
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[1] 8
arg_in:  1 buffer int[1] NULL

[test]
name: threads
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[1] 8
arg_in:  1 buffer uint[1] NULL

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *out, local TYPE *mem, global TYPE *initial, global TYPE *compare, global TYPE *value) { \
	mem[0] = initial[0]; \
	mem[1] = initial[1]; \
	TYPE a = atomic_cmpxchg(mem, compare[0], value[0]); \
	out[0] = a; \
	out[1] = *mem; \
	a = atomic_cmpxchg(mem+1, compare[1], value[1]); \
	out[2] = a; \
	out[3] = mem[1]; \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out, local TYPE *mem) { \
	int i; \
	*mem = 0; \
	barrier(CLK_LOCAL_MEM_FENCE); \
	TYPE id = get_local_id(0); \
	for(i = 0; i < get_local_size(0); i++){ \
		if (i == id){ \
			atomic_cmpxchg(mem, id, id+1); \
		} \
		barrier(CLK_LOCAL_MEM_FENCE); \
	} \
	*out = *mem; \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)

THREADS_TEST(int)
THREADS_TEST(uint)
