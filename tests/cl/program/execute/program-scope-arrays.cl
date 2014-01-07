/*!
[config]
name: program-scope-arrays
dimensions: 1
global_size: 4 0 0


[test]
name: simple-constant
kernel_name: simple_constant
arg_out: 0 buffer float[4] 3.0 3.0 3.0 3.0

[test]
name: given-constant
kernel_name: given_constant
arg_in: 1 int 1
arg_out: 0 buffer float[4] 3.0 3.0 3.0 3.0

[test]
name: simple-gid
kernel_name: simple_gid
arg_out: 0 buffer float[4] 4.0 3.0 2.0 1.0

[test]
name: indirection
kernel_name: indirection
arg_in: 1 buffer uchar[4] 0 1 2 3
arg_out: 0 buffer float[4] 4.0 3.0 2.0 1.0

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
