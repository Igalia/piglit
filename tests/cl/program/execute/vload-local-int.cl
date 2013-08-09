/*!
[config]
name: local_memory

dimensions: 1
global_size: 1 0 0
local_size:  1 0 0

[test]
name: Scalar (sanity) test
kernel_name: scalar_test
arg_out: 0 buffer int[1] 1


[test]
name: vload2 local
kernel_name: vecload2
arg_out: 0 buffer int[2] 1 2

[test]
name: vload3 local
kernel_name: vecload3
arg_out: 0 buffer int[3] 1 2 3

[test]
name: vload4 local
kernel_name: vecload4
arg_out: 0 buffer int[4] 1 2 3 4

[test]
name: vload8 local
kernel_name: vecload8
arg_out: 0 buffer int[8] 1 2 3 4 5 6 7 8

[test]
name: vload16 local
kernel_name: vecload16
arg_out: 0 buffer int[16] 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16


!*/

kernel void scalar_test(global int* out){
	local int local_mem[1];
	local_mem[0] = 1;
	out[0] = local_mem[0];
}

kernel void vecload2(global int *out) {
	local int local_mem[2];
	local_mem[0] = 1;
	local_mem[1] = 2;
	vstore2(vload2(0, local_mem), 0, out);
}

kernel void vecload3(global int *out) {
	local int local_mem[3];
	local_mem[0] = 1;
	local_mem[1] = 2;
	local_mem[2] = 3;
	vstore3(vload3(0, local_mem), 0, out);
}

kernel void vecload4(global int *out) {
	local int local_mem[4];
	local_mem[0] = 1;
	local_mem[1] = 2;
	local_mem[2] = 3;
	local_mem[3] = 4;
	vstore4(vload4(0, local_mem), 0, out);
}

kernel void vecload8(global int *out) {
	local int local_mem[8];
	local_mem[0] = 1;
	local_mem[1] = 2;
	local_mem[2] = 3;
	local_mem[3] = 4;
	local_mem[4] = 5;
	local_mem[5] = 6;
	local_mem[6] = 7;
	local_mem[7] = 8;
	vstore8(vload8(0, local_mem), 0, out);
}

kernel void vecload16(global int *out) {
	local int local_mem[16];
	local_mem[0] = 1;
	local_mem[1] = 2;
	local_mem[2] = 3;
	local_mem[3] = 4;
	local_mem[4] = 5;
	local_mem[5] = 6;
	local_mem[6] = 7;
	local_mem[7] = 8;
	local_mem[8] = 9;
	local_mem[9] = 10;
	local_mem[10] = 11;
	local_mem[11] = 12;
	local_mem[12] = 13;
	local_mem[13] = 14;
	local_mem[14] = 15;
	local_mem[15] = 16;
	vstore16(vload16(0, local_mem), 0, out);
}
