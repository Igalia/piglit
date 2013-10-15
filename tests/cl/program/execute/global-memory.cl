/*!
[config]
name: global_memory

[test]
name: Simple
kernel_name: simple
dimensions: 1
global_size: 1 0 0
local_size:  1 0 0
arg_out: 0 buffer int[2] -1 -1
arg_in: 1 buffer int[2] 0 0

[test]
name: (16 x 1 x 1) (16 x 1 x 1)
kernel_name: global_memory_one_work_group
dimensions: 1
global_size: 16 0 0
local_size:  16 0 0
arg_out: 0 buffer int[16] 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 0
arg_in: 1 buffer int[16] repeat 0

[test]
name: (64 x 1 x 1) (4 x 1 x 1)
kernel_name: global_memory_many_work_groups
dimensions: 1
global_size: 64 0 0
local_size:   4 0 0
arg_out: 0 buffer int[64] repeat 1 2 3 0
arg_in: 1 buffer int[64] repeat 0

[test]
name: 2 global memory objects
kernel_name: global_memory_two_objects
dimensions: 1
global_size: 4 0 0
local_size:  4 0 0
arg_out: 0 buffer int[8] 3 2 1 0 6 4 2 0
arg_in: 1 buffer int[4] repeat 0
arg_in: 2 buffer int[4] repeat 0

!*/

kernel void simple(global int *out, global int *tmp_mem) {
	tmp_mem[0] = 0xffffffff;
	tmp_mem[1] = 0xffffffff;
	out[0] = tmp_mem[0];
	out[1] = tmp_mem[1];
}

kernel void global_memory_one_work_group(global int *out, global int *tmp_mem) {
	int group_offset = get_group_id(0) * get_local_size(0);
	int index = get_local_id(0);
	int index2;
	tmp_mem[group_offset + index] = index;
	index2 = index + 1;
	index2 = (index2 % 16);
	barrier(CLK_GLOBAL_MEM_FENCE);
	out[index] = tmp_mem[group_offset + index2];
}

kernel void global_memory_many_work_groups(global int *out, global int *tmp_mem) {
	int group_offset = get_group_id(0) * get_local_size(0);
	int index = get_local_id(0);
	int global_id = get_global_id(0);
	int index2;
	tmp_mem[group_offset + index] = index;
	index2 = index + 1;
	index2 = (index2 % 4);
	barrier(CLK_GLOBAL_MEM_FENCE);
	out[global_id] = tmp_mem[group_offset + index2];
}

kernel void global_memory_two_objects(global int *out, global int *tmp_mem0, global int *tmp_mem1) {
	int group_offset = get_group_id(0) * get_local_size(0);
	int index = get_local_id(0);
	int index2 = 3 - index;
	tmp_mem0[group_offset + index] = index;
	tmp_mem1[group_offset + index] = index * 2;
	barrier(CLK_GLOBAL_MEM_FENCE);
	out[group_offset + index] = tmp_mem0[group_offset + index2];
	out[group_offset + index + 4] = tmp_mem1[group_offset + index2];
}
