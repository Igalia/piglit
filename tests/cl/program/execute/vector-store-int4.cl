/*!
[config]
name: int4_store
clc_version_min: 10
kernel_name: global_store

[test]
name: Global Address Space
arg_out: 0 buffer int4[2] 1 2 3 4 5 6 7 8
arg_in:  1 buffer int4[2] 1 2 3 4 5 6 7 8
!*/

kernel void global_store(global int4* out, global int4* in) {
	out[0] = in[0];
	out[1] = in[1];
}
