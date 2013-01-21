/*!
[config]
name: i32 stack array
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

##===----------------------------------------------------------------------===##
# INDIRECT READ
##===----------------------------------------------------------------------===##

[test]
kernel_name: stack_array_indirect_read
name: indirect read 0
arg_out: 0 buffer int3[2] 4 5 6 7 8 9
arg_in:  1 buffer int[2] 0 1

[test]
name: indirect read 1
kernel_name: stack_array_indirect_read
arg_out: 0 buffer int3[2] 7 8 9 4 5 6
arg_in:  1 buffer int[2] 1 0

##===----------------------------------------------------------------------===##
# INDIRECT WRITE
##===----------------------------------------------------------------------===##

[test]
name: indirect write 0
kernel_name: stack_array_indirect_read
arg_out: 0 buffer int3[2] 4 5 6 7 8 9
arg_in:  1 buffer int[2] 0 1

[test]
name: indirect write 1
kernel_name: stack_array_indirect_read
arg_out: 0 buffer int3[2] 7 8 9 4 5 6
arg_in:  1 buffer int[2] 1 0


!*/

kernel void stack_array_indirect_read(global int3* out, global int *in) {
	int3 stack[2];
	stack[0].s0 = 4;
	stack[0].s1 = 5;
	stack[0].s2 = 6;
	stack[1].s0 = 7;
	stack[1].s1 = 8;
	stack[1].s2 = 9;

	out[0] = stack[in[0]];
	out[1] = stack[in[1]];
}

kernel void stack_array_indirect_write(global int3* out, global int *in) {
	int3 stack[2];
	stack[in[0]].s0 = 4;
	stack[in[0]].s1 = 5;
	stack[in[0]].s2 = 6;
	stack[in[1]].s0 = 7;
	stack[in[1]].s1 = 8;
	stack[in[1]].s2 = 9;

	out[0] = stack[0];
	out[1] = stack[1];
}
