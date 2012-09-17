/*!
[config]
name: Scalar load uchar
clc_version_min: 11
kernel_name: uchar_load

[test]
name: global address space
arg_out: 0 buffer uint[1] 5
arg_in:  1 buffer uchar[1] 5
!*/

kernel void uchar_load(global uint *out, global uchar *in) {
	out[0] = in[0];
}
