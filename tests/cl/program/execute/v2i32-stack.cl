/*!
[config]

[test]
kernel_name: direct_write_indirect_read
name: direct write - indirect read 0
arg_out: 0 buffer int2[1] 0 1
arg_in:  1 int 0

[test]
kernel_name: direct_write_indirect_read
name: direct write - indirect read 1
arg_out: 0 buffer int2[1] 2 3
arg_in:  1 int 1

!*/

kernel void direct_write_indirect_read(global int2 *out, int index) {

	int2 stack[2];
	stack[0].s0 = 0;
	stack[0].s1 = 1;

	stack[1].s0 = 2;
	stack[1].s1 = 3;

	out[0] = stack[index];
}
