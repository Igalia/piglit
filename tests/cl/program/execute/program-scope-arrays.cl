/*!
[config]
name: program-scope-arrays
dimensions: 1
global_size: 4 0 0


[test]
name: simple-constant-char
kernel_name: simple_constant_char
arg_out: 0 buffer char[4] 3 3 3 3

[test]
name: given-constant-char
kernel_name: given_constant_char
arg_in: 1 int 2
arg_out: 0 buffer char[4] 2 2 2 2

[test]
name: simple-gid-char
kernel_name: simple_gid_char
arg_out: 0 buffer char[4] 4 3 2 1

[test]
name: indirection-char
kernel_name: indirection_char
arg_in: 1 buffer uchar[4] 3 2 1 0
arg_out: 0 buffer char[4] 1 2 3 4

[test]
name: simple-constant
kernel_name: simple_constant
arg_out: 0 buffer float[4] 3.0 3.0 3.0 3.0

[test]
name: given-constant
kernel_name: given_constant
arg_in: 1 int 2
arg_out: 0 buffer float[4] 2.0 2.0 2.0 2.0

[test]
name: simple-gid
kernel_name: simple_gid
arg_out: 0 buffer float[4] 4.0 3.0 2.0 1.0

[test]
name: indirection
kernel_name: indirection
arg_in: 1 buffer uchar[4] 3 2 1 0
arg_out: 0 buffer float[4] 1.0 2.0 3.0 4.0


[test]
name: simple-constant-2
kernel_name: simple_constant_2
arg_out: 0 buffer float[8] 3.0 5.0 3.0 5.0 3.0 5.0 3.0 5.0

[test]
name: given-constant-2
kernel_name: given_constant_2
arg_in: 1 int 2
arg_out: 0 buffer float[8] 2.0 5.0 2.0 5.0 2.0 5.0 2.0 5.0

[test]
name: simple-gid-2
kernel_name: simple_gid_2
arg_out: 0 buffer float[8] 4.0 5.0 3.0 5.0 2.0 5.0 1.0 5.0

[test]
name: indirection-2
kernel_name: indirection_2
arg_in: 1 buffer uchar[4] 3 2 1 0
arg_out: 0 buffer float[8] 1.0 5.0 2.0 5.0 3.0 5.0 4.0 5.0

!*/

__constant float arr[] = {
4.0f,
3.0f,
2.0f,
1.0f,
};

__kernel void simple_constant(__global float *out) {
	int i = get_global_id(0);
	out[i] = arr[1];
}

__kernel void given_constant(__global float *out, int c) {
	int i = get_global_id(0);
	out[i] = arr[c];
}

__kernel void simple_gid(__global float *out) {
	int i = get_global_id(0);
	out[i] = arr[i];
}

__kernel void indirection(__global float *out, __global uchar *in) {
	int i = get_global_id(0);
	out[i] = arr[in[i]];
}

__constant char arrc[] = { 4, 3, 2, 1, };

__kernel void simple_constant_char(__global char *out) {
	int i = get_global_id(0);
	out[i] = arrc[1];
}

__kernel void given_constant_char(__global char *out, int c) {
	int i = get_global_id(0);
	out[i] = arrc[c];
}

__kernel void simple_gid_char(__global char *out) {
	int i = get_global_id(0);
	out[i] = arrc[i];
}

__kernel void indirection_char(__global char *out, __global uchar *in) {
	int i = get_global_id(0);
	out[i] = arrc[in[i]];
}

__constant float2 arr2[] = {
{4.0f, 5.0f},
{3.0f, 5.0f},
{2.0f, 5.0f},
{1.0f, 5.0f}
};

__kernel void simple_constant_2(__global float2 *out) {
	int i = get_global_id(0);
	out[i] = arr2[1];
}

__kernel void given_constant_2(__global float2 *out, int c) {
	int i = get_global_id(0);
	out[i] = arr2[c];
}

__kernel void simple_gid_2(__global float2 *out) {
	int i = get_global_id(0);
	out[i] = arr2[i];
}

__kernel void indirection_2(__global float2 *out, __global uchar *in) {
	int i = get_global_id(0);
	out[i] = arr2[in[i]];
}
