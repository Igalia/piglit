/*!
[config]
name: constant vload int4
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

[test]
name: vector load2
kernel_name: vecload2
arg_out: 0 buffer int[2] 1 2
arg_in: 1 buffer int[2] 1 2

[test]
name: vector load3
kernel_name: vecload3
arg_out: 0 buffer int[3] 1 2 3
arg_in: 1 buffer int[3] 1 2 3

[test]
name: vector load4
kernel_name: vecload4
arg_out: 0 buffer int[4] 1 2 3 4
arg_in: 1 buffer int[4] 1 2 3 4

[test]
name: vector load8
kernel_name: vecload8
arg_out: 0 buffer int[8] 1 2 3 4 5 6 7 8
arg_in: 1 buffer int[8] 1 2 3 4 5 6 7 8

[test]
name: vector load16
kernel_name: vecload16
arg_out: 0 buffer int[16] 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
arg_in: 1 buffer int[16] 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16


!*/

kernel void load1(global int* out, constant int* input) {
	out[0] = input[0];
}

kernel void vecload2(global int* out, constant int* input) {
	vstore2(vload2(0, input), 0, out);
}

kernel void vecload3(global int* out, constant int* input) {
	vstore3(vload3(0, input), 0, out);
}

kernel void vecload4(global int* out, constant int* input) {
	vstore4(vload4(0, input), 0, out);
}

kernel void vecload8(global int* out, constant int* input) {
	vstore8(vload8(0, input), 0, out);
}

kernel void vecload16(global int* out, constant int* input) {
	vstore16(vload16(0, input), 0, out);
}
