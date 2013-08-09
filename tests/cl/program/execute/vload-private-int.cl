/*!
[config]
name: private_memory

dimensions: 1
global_size: 1 0 0
local_size:  1 0 0

[test]
name: Scalar (sanity) test
kernel_name: scalar_test
arg_out: 0 buffer int[1] 1


[test]
name: vload2 private
kernel_name: vecload2
arg_out: 0 buffer int[2] 1 2

[test]
name: vload3 private
kernel_name: vecload3
arg_out: 0 buffer int[3] 1 2 3

[test]
name: vload4 private
kernel_name: vecload4
arg_out: 0 buffer int[4] 1 2 3 4

[test]
name: vload8 private
kernel_name: vecload8
arg_out: 0 buffer int[8] 1 2 3 4 5 6 7 8

[test]
name: vload16 private
kernel_name: vecload16
arg_out: 0 buffer int[16] 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16


!*/

kernel void scalar_test(global int* out){
	private int private_mem[1];
	private_mem[0] = 1;
	out[0] = private_mem[0];
}

kernel void vecload2(global int *out) {
	private int private_mem[2];
	private_mem[0] = 1;
	private_mem[1] = 2;
	vstore2(vload2(0, private_mem), 0, out);
}

kernel void vecload3(global int *out) {
	private int private_mem[3];
	private_mem[0] = 1;
	private_mem[1] = 2;
	private_mem[2] = 3;
	vstore3(vload3(0, private_mem), 0, out);
}

kernel void vecload4(global int *out) {
	private int private_mem[4];
	private_mem[0] = 1;
	private_mem[1] = 2;
	private_mem[2] = 3;
	private_mem[3] = 4;
	vstore4(vload4(0, private_mem), 0, out);
}

kernel void vecload8(global int *out) {
	private int private_mem[8];
	private_mem[0] = 1;
	private_mem[1] = 2;
	private_mem[2] = 3;
	private_mem[3] = 4;
	private_mem[4] = 5;
	private_mem[5] = 6;
	private_mem[6] = 7;
	private_mem[7] = 8;
	vstore8(vload8(0, private_mem), 0, out);
}

kernel void vecload16(global int *out) {
	private int private_mem[16];
	private_mem[0] = 1;
	private_mem[1] = 2;
	private_mem[2] = 3;
	private_mem[3] = 4;
	private_mem[4] = 5;
	private_mem[5] = 6;
	private_mem[6] = 7;
	private_mem[7] = 8;
	private_mem[8] = 9;
	private_mem[9] = 10;
	private_mem[10] = 11;
	private_mem[11] = 12;
	private_mem[12] = 13;
	private_mem[13] = 14;
	private_mem[14] = 15;
	private_mem[15] = 16;
	vstore16(vload16(0, private_mem), 0, out);
}
