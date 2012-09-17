/*!
[config]
name: Scalar load float
clc_version_min: 11

[test]
kernel_name: load_global
name: global address space
arg_out: 0 buffer float[1] 3.5
arg_in:  1 buffer float[1] 3.5

[test]
kernel_name: load_param
name: parameter address space
arg_out: 0 buffer float[1] 3.6
arg_in:  1  float 3.6

[test]
kernel_name: load_constant
name: constant address space
arg_out: 0 buffer float[1] 3.7
arg_in:  1 buffer float[1] 3.7
!*/

kernel void load_global(global float *out, global float *in) {
	out[0] = in[0];
}

kernel void load_param(global float *out, float in) {
	out[0] = in;
}

kernel void load_constant(global float *out, constant float *in) {
	out[0] = in[0];
}
