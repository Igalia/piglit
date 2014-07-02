/*!
[config]
name: __attribute__ tests
clc_version_min: 10

[test]
name: work_group_size_hint
kernel_name: testKernel
dimensions: 1
global_size: 4 0 0
arg_out: 0 buffer int[4] repeat 5

!*/

/*
https://bugs.freedesktop.org/show_bug.cgi?id=76223

If you remove the __attribute__((work_group_size_hint(4,1,1)), then the test works.

With the attribute, you get:
LLVM ERROR: not a number, or does not fit in an unsigned int

This only causes an error if there's more than 1 kernel in the program
*/
__kernel __attribute__((work_group_size_hint(4, 1, 1))) void testKernel(
	global int* out
) {
	const size_t gid = get_global_id(0);
	if (gid >= get_global_size(0))
		return;
	out[gid] = 5;
}

/*
XXX: With this useless kernel commented out, things work...
*/
__kernel void BogusKernel(global int* out, global int* in){
	*out = *in;
}
