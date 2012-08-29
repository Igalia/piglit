/*!
[config]
name: Vector load int4
clc_version_min: 10

kernel_name: vecload
dimensions: 1
global_size: 1 0 0

[test]
name: vector load
arg_out: 0 buffer int[8] 1 2 3 4 \
                         1 2 3 4
arg_in: 1 int4 1 2 3 4
!*/

kernel void vecload(global int* out, int4 vec) {
	out[0] = vec.s0;
	out[1] = vec.s1;
	out[2] = vec.s2;
	out[3] = vec.s3;

	out[4] = vec.x;
	out[5] = vec.y;
	out[6] = vec.z;
	out[7] = vec.w;
}
