/*!
[config]
name: Scalar load uchar

[test]
name: global address space
kernel_name: uchar_load_global
arg_out: 0 buffer uint[1] 5
arg_in:  1 buffer uchar[1] 5

[test]
name: parameter address space
kernel_name: uchar_load_param
arg_out: 0 buffer uint[1] 6
arg_in:  1 uchar 6

[test]
name: constant address space
kernel_name: uchar_load_constant
arg_out: 0 buffer uint[1] 7
arg_in:  1 buffer uchar[1] 7

!*/

kernel void uchar_load_global(global uint *out, global uchar *in) {
	out[0] = in[0];
}

kernel void uchar_load_param(global uint *out, uchar in) {
	out[0] = in;
}

kernel void uchar_load_constant(global uint *out, constant uchar *in) {
	out[0] = in[0];
}
