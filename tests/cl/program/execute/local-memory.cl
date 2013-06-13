/*!
[config]
name: local_memory

[test]
name: Simple
kernel_name: simple
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[2] -1 -1

[test]
name: (16 x 1 x 1) (16 x 1 x 1)
kernel_name: local_memory_one_work_group
dimensions: 1
global_size: 16 0 0
local_size:  16 0 0
arg_out: 0 buffer int[16] 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 0

[test]
name: (64 x 1 x 1) (4 x 1 x 1)
kernel_name: local_memory_many_work_groups
dimensions: 1
global_size: 64 0 0
local_size:   4 0 0
arg_out: 0 buffer int[64] repeat 1 2 3 0

[test]
name: 2 local memory objects
kernel_name: local_memory_two_objects
dimensions: 1
global_size: 4 0 0
local_size:  4 0 0
arg_out: 0 buffer int[8] 3 2 1 0 6 4 2 0

!*/

kernel void simple(global int *out) {
	local volatile int local_mem[2];
	local_mem[0] = 0xffffffff;
	local_mem[1] = 0xffffffff;
	out[0] = local_mem[0];
	out[1] = local_mem[1];
}

kernel void local_memory_one_work_group(global int *out) {
	local int local_mem[16];
	int index = get_local_id(0);
	int index2;
	local_mem[index] = index;
	index2 = index + 1;
	index2 = index2 == 16 ? 0 : index2;
	barrier(CLK_LOCAL_MEM_FENCE);
	out[index] = local_mem[index2];
}

kernel void local_memory_many_work_groups(global int *out) {
	local int local_mem[4];
	int index = get_local_id(0);
	int global_id = get_global_id(0);
	int index2;
	local_mem[index] = index;
	index2 = index + 1;
	index2 = index2 == 4 ? 0 : index2;
	barrier(CLK_LOCAL_MEM_FENCE);
	out[global_id] = local_mem[index2];
}

kernel void local_memory_two_objects(global int *out) {
	local int local_mem0[4];
	local int local_mem1[4];

	int index = get_local_id(0);
	local_mem0[index] = index;
	local_mem1[index] = index * 2;
	int index2 = 3 - index;
	barrier(CLK_LOCAL_MEM_FENCE);
	out[index] = local_mem0[index2];
	out[index + 4] = local_mem1[index2];
}
