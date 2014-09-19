/*!
[config]
name: i32 stack array
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

# We want to test with an array of 5 elements, so that on GPUs with vec4
# registers, the array will be span two registers.

##===----------------------------------------------------------------------===##
# DIRECT WRITE / INDIRECT READ
##===----------------------------------------------------------------------===##

[test]
kernel_name: stack_array_read
name: i32 stack array read up
arg_out: 0 buffer int[5] 4 5 6 7 8
arg_in:  1 buffer int[5] 0 1 2 3 4

[test]
kernel_name: stack_array_read
name: i32 stack array read const
arg_out: 0 buffer int[5] 5 5 5 5 5
arg_in:  1 buffer int[5] 1 1 1 1 1

[test]
kernel_name: stack_array_read
name: i32 stack array read down
arg_out: 0 buffer int[5] 8 7 5 6 4
arg_in:  1 buffer int[5] 4 3 1 2 0

[test]
kernel_name: stack_array_read
name: i32 stack array read rand
arg_out: 0 buffer int[5] 7 5 8 4 6
arg_in:  1 buffer int[5] 3 1 4 0 2

##===----------------------------------------------------------------------===##
# INDIRECT WRITE / DIRECT READ
##===----------------------------------------------------------------------===##

[test]
kernel_name: stack_array_write
name: i32 stack array write
arg_out: 0 buffer int[5] 4 5 6 7 8
arg_in:  1 buffer int[5] 0 1 2 3 4

##===----------------------------------------------------------------------===##
# INDIRECT WRITE / DIRECT READ
##===----------------------------------------------------------------------===##

[test]
kernel_name: stack_array_write_read
name: i32 stack array read write
arg_out: 0 buffer int[5] 4 5 6 7 8
arg_in:  1 buffer int[10] 0 1 2 3 4 \
                          0 1 2 3 4

##===----------------------------------------------------------------------===##
# DIRECT WRITE (IF and ELSE) / DIRECT READ
##===----------------------------------------------------------------------===##
[test]
kernel_name: stack_array_write_if_else_read
name: i32 stack array direct write (IF and ELSE)
arg_out: 0 buffer int[5] 4 5 6 7 8
arg_in:  1 buffer int[1] 1

##===----------------------------------------------------------------------===##
# DIRECT WRITE (IF and ELSE) / INDIRECT READ
##===----------------------------------------------------------------------===##
[test]
kernel_name: stack_array_write_if_else_indirect_read
name: i32 stack array direct write (IF and ELSE) indirect read up
arg_out: 0 buffer int[5] 4 5 6 7 8
arg_in:  1 buffer int[6] 1 \
                         0 1 2 3 4

[test]
kernel_name: stack_array_write_if_else_indirect_read
name: i32 stack array direct write (IF and ELSE) indirect read const
arg_out: 0 buffer int[5] 0 0 0 0 0
arg_in:  1 buffer int[6] 0 \
                         0 1 2 3 4

[test]
kernel_name: stack_array_write_if_else_indirect_read
name: i32 stack array direct write (IF and ELSE) indirect read down
arg_out: 0 buffer int[5] 8 7 6 5 4
arg_in:  1 buffer int[6] 1 \
                         4 3 2 1 0

##===----------------------------------------------------------------------===##
# INDIRECT WRITE (IF and ELSE) / DIRECT READ
##===----------------------------------------------------------------------===##
[test]
kernel_name: stack_array_indirect_write_if_else_read
name: i32 stack array indirect write (IF and ELSE) direct read
arg_out: 0 buffer int[5] 4 5 6 7 8
arg_in:  1 buffer int[6] 1 \
                         0 1 2 3 4

##===----------------------------------------------------------------------===##
# INDIRECT WRITE (IF and ELSE) / INDIRECT READ
##===----------------------------------------------------------------------===##
[test]
kernel_name: stack_array_indirect_write_if_else_indirect_read
name: i32 stack array indirect write (IF and ELSE) indirect read up
arg_out: 0 buffer int[5] 4 5 6 7 8
arg_in:  1 buffer int[11] 1 \
                          0 1 2 3 4 \
                          0 1 2 3 4
[test]
kernel_name: stack_array_indirect_write_if_else_indirect_read
name: i32 stack array indirect write (IF and ELSE) indirect read up-down
arg_out: 0 buffer int[5] 8 7 6 5 4
arg_in:  1 buffer int[11] 1 \
                          0 1 2 3 4 \
                          4 3 2 1 0
!*/

kernel void stack_array_read(global int* out, global int *in) {

	int stack[5];
	stack[0] = 4;
	stack[1] = 5;
	stack[2] = 6;
	stack[3] = 7;
	stack[4] = 8;
	out[0] = stack[in[0]];
	out[1] = stack[in[1]];
	out[2] = stack[in[2]];
	out[3] = stack[in[3]];
	out[4] = stack[in[4]];
}

kernel void stack_array_write(global int* out, global int *in) {

	int stack[5];
	stack[in[0]] = 4;
	stack[in[1]] = 5;
	stack[in[2]] = 6;
	stack[in[3]] = 7;
	stack[in[4]] = 8;

	out[0] = stack[0];
	out[1] = stack[1];
	out[2] = stack[2];
	out[3] = stack[3];
	out[4] = stack[4];
}

kernel void stack_array_write_read(global int *out, global int *in) {

	int stack[5];
	stack[in[0]] = 4;
	stack[in[1]] = 5;
	stack[in[2]] = 6;
	stack[in[3]] = 7;
	stack[in[4]] = 8;

	out[0] = stack[in[5]];
	out[1] = stack[in[6]];
	out[2] = stack[in[7]];
	out[3] = stack[in[8]];
	out[4] = stack[in[9]];

}

kernel void stack_array_write_if_else_read(global int *out, global int *in) {

	int stack[5];
	if (in[0] == 1) {
		stack[0] = 4;
		stack[1] = 5;
		stack[2] = 6;
		stack[3] = 7;
		stack[4] = 8;
	} else {
		stack[0] = stack[1] = stack[2] = stack[3] = stack[4] = 0;
	}

	out[0] = stack[0];
	out[1] = stack[1];
	out[2] = stack[2];
	out[3] = stack[3];
	out[4] = stack[4];
}

kernel void stack_array_write_if_else_indirect_read(global int *out, global int *in) {

	int stack[5];
	if (in[0] == 1) {
		stack[0] = 4;
		stack[1] = 5;
		stack[2] = 6;
		stack[3] = 7;
		stack[4] = 8;
	} else {
		stack[0] = stack[1] = stack[2] = stack[3] = stack[4] = 0;
	}

	out[0] = stack[in[1]];
	out[1] = stack[in[2]];
	out[2] = stack[in[3]];
	out[3] = stack[in[4]];
	out[4] = stack[in[5]];
}

kernel void stack_array_indirect_write_if_else_read(global int *out, global int *in) {

	int stack[5];
	if (in[0] == 1) {
		stack[in[1]] = 4;
		stack[in[2]] = 5;
		stack[in[3]] = 6;
		stack[in[4]] = 7;
		stack[in[5]] = 8;
	} else {
		stack[in[1]] = 9;
		stack[in[2]] = 10;
		stack[in[3]] = 11;
		stack[in[4]] = 12;
		stack[in[5]] = 13;
	}

	out[0] = stack[0];
	out[1] = stack[1];
	out[2] = stack[2];
	out[3] = stack[3];
	out[4] = stack[4];
}

kernel void stack_array_indirect_write_if_else_indirect_read(global int *out, global int *in) {
	int stack[5];
	if (in[0] == 1) {
		stack[in[1]] = 4;
		stack[in[2]] = 5;
		stack[in[3]] = 6;
		stack[in[4]] = 7;
		stack[in[5]] = 8;
	} else {
		stack[in[1]] = 9;
		stack[in[2]] = 10;
		stack[in[3]] = 11;
		stack[in[4]] = 12;
		stack[in[5]] = 13;
	}

	out[0] = stack[in[6]];
	out[1] = stack[in[7]];
	out[2] = stack[in[8]];
	out[3] = stack[in[9]];
	out[4] = stack[in[10]];
}
