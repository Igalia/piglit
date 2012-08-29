/*!
[config]
name: Comma operator
clc_version_min: 10

[test]
name: comma
kernel_name: comma
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[6] repeat 1
!*/

kernel void comma(global int* out) {
	out[0] = (0, 0, 1);
	out[1] = 1, 0, 0;
	out[3] = (out[2] = 1, 1);
	out[5] = out[4] = 1, 0;
}
