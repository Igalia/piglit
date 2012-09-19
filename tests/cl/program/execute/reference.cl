/*!
[config]
name: Reference and dereferance operators
clc_version_min: 10

[test]
name: ref
kernel_name: ref
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[3] repeat 5
!*/

kernel void ref(global int* out) {
	int a = 5;
	int* b = &a;

	out[0] = *(&a);
	out[1] = *b;
	out[2] = *out;
}
