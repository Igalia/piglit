/*!
[config]
name: atomic_xchg local
clc_version_min: 11

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[2] -4 5
arg_in:  1 buffer int[1] NULL
arg_in:  2 int           -4
arg_in:  3 int           5

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer uint[2] 4 5
arg_in:  1 buffer uint[1] NULL
arg_in:  2 uint           4
arg_in:  3 uint           5

[test]
name: simple float
kernel_name: simple_float
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer float[2] 4.0 5.0
arg_in:  1 buffer float[1] NULL
arg_in:  2 float           4.0
arg_in:  3 float           5.0

[test]
name: threads
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[1] 7
arg_in:  1 buffer int[1] NULL

[test]
name: threads
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[1] 7
arg_in:  1 buffer uint[1] NULL

[test]
name: threads
kernel_name: threads_float
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer float[1] 7.0
arg_in:  1 buffer float[1] NULL

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *out, local TYPE *mem, TYPE initial, TYPE value) { \
	*mem = initial; \
	TYPE a = atomic_xchg(mem, value); \
	out[0] = a; \
	out[1] = *mem; \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out, local TYPE *mem) { \
	int i; \
	*mem = 0; \
	barrier(CLK_LOCAL_MEM_FENCE); \
	TYPE id = get_local_id(0); \
	for(i = 0; i < get_local_size(0); i++){ \
		if (i == id){ \
			atomic_xchg(mem, (TYPE)id); \
		} \
		barrier(CLK_LOCAL_MEM_FENCE); \
	} \
	*out = *mem; \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)
SIMPLE_TEST(float)

THREADS_TEST(int)
THREADS_TEST(uint)
THREADS_TEST(float)
