/*!
[config]
name: atomic_xchg global
clc_version_min: 11

[test]
name: simple int
kernel_name: simple_int
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] -4
arg_in:  0 buffer int[1] 1
arg_in:  1 int           -4

[test]
name: simple uint
kernel_name: simple_uint
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 4
arg_in:  0 buffer int[1] 2
arg_in:  1 int           4

[test]
name: simple float
kernel_name: simple_float
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer float[1] 4.0
arg_in:  0 buffer float[1] 3.0
arg_in:  1 float           4.0

[test]
name: threads int
kernel_name: threads_int
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer int[1] 7
arg_in:  0 buffer int[1] 0

[test]
name: threads uint
kernel_name: threads_uint
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer uint[1] 7
arg_in:  0 buffer uint[1] 0

[test]
name: threads float
kernel_name: threads_float
dimensions: 1
global_size: 8 0 0
local_size:  8 0 0
arg_out: 0 buffer float[1] 7.0
arg_in:  0 buffer float[1] 0.0

!*/

#define SIMPLE_TEST(TYPE) \
kernel void simple_##TYPE(global TYPE *out, TYPE value) { \
	atomic_xchg(out, value); \
}

#define THREADS_TEST(TYPE) \
kernel void threads_##TYPE(global TYPE *out) { \
	int i; \
	TYPE id = get_global_id(0); \
	barrier(CLK_GLOBAL_MEM_FENCE); \
	for(i = 0; i < get_global_size(0); i++){ \
		if (i == id){ \
			atomic_xchg(out, (TYPE)id); \
		} \
		barrier(CLK_GLOBAL_MEM_FENCE); \
	} \
}

SIMPLE_TEST(int)
SIMPLE_TEST(uint)
SIMPLE_TEST(float)

THREADS_TEST(int)
THREADS_TEST(uint)
THREADS_TEST(float)
