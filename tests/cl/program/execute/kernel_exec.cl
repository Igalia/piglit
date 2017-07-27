/*!
[config]
name: kernel_exec macro tests
clc_version_min: 10

[test]
name: kernel_exec
kernel_name: testKernel
dimensions: 1
global_size: 4 0 0
arg_out: 0 buffer int2[4] repeat 5

[test]
name: __kernel_exec
kernel_name: test__Kernel
dimensions: 1
global_size: 4 0 0
arg_out: 0 buffer int3[4] repeat 7

!*/

kernel_exec(4, int2) void testKernel(
	global int2* out
) {
	const size_t gid = get_global_id(0);
	if (gid >= get_global_size(0))
		return;
	out[gid] = (int2){5, 5};
}

__kernel_exec(4, int3) void test__Kernel(
	global int3* out
) {
	const size_t gid = get_global_id(0);
	if (gid >= get_global_size(0))
		return;
	out[gid] = (int3){7, 7, 7};
}
