/*!
[config]

[test]
kernel_name: direct_write_indirect_read
name: direct write - indirect read 0
arg_out: 0 buffer int4[1] 0 1 2 3
arg_in:  1 int 0

[test]
kernel_name: direct_write_indirect_read
name: direct write - indirect read 1
arg_out: 0 buffer int4[1] 4 5 6 7
arg_in:  1 int 1
!*/

kernel void direct_write_indirect_read(global int4 *out, int index) {

	int4 stack[2];
	stack[0].s0 = 0;
	stack[0].s1 = 1;
	stack[0].s2 = 2;
	stack[0].s3 = 3;

	stack[1].s0 = 4;
	stack[1].s1 = 5;
	stack[1].s2 = 6;
	stack[1].s3 = 7;

	out[0] = stack[index];
}
