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

[test]
name: simple-constant-struct
kernel_name: simple_constant_struct
arg_out: 0 buffer float[4] 32.0 32.0 32.0 32.0
arg_out: 1 buffer int[4] 42 42 42 42

[test]
name: given-constant-struct
kernel_name: given_constant_struct
arg_in: 2 int 2
arg_out: 0 buffer float[4] 33.0 33.0 33.0 33.0
arg_out: 1 buffer int[4] 43 43 43 43

[test]
name: simple-gid-struct
kernel_name: simple_gid_struct
arg_out: 0 buffer float[4] 31.0 32.0 33.0 34.0
arg_out: 1 buffer int[4] 41 42 43 44

[test]
name: indirection-struct
kernel_name: indirection_struct
arg_in: 2 buffer uchar[4] 3 2 1 0
arg_out: 0 buffer float[4] 34.0 33.0 32.0 31.0
arg_out: 1 buffer int[4] 44 43 42 41

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

__constant struct foo {
	float a;
	int b;
} arrs[] = {
	{31.0, 41},
	{32.0, 42},
	{33.0, 43},
	{34.0, 44}
};

__kernel void simple_constant_struct(__global float *outa, __global int *outb) {
	int i = get_global_id(0);
	outa[i] = arrs[1].a;
	outb[i] = arrs[1].b;
}

__kernel void given_constant_struct(__global float *outa, __global int *outb, int c) {
	int i = get_global_id(0);
	outa[i] = arrs[c].a;
	outb[i] = arrs[c].b;
}

__kernel void simple_gid_struct(__global float *outa, __global int *outb) {
	int i = get_global_id(0);
	outa[i] = arrs[i].a;
	outb[i] = arrs[i].b;
}

__kernel void indirection_struct(__global float *outa, __global int *outb, __global uchar *in) {
	int i = get_global_id(0);
	outa[i] = arrs[in[i]].a;
	outb[i] = arrs[in[i]].b;
}
