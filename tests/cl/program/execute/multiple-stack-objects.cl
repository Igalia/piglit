/*!
[config]

[test]
kernel_name: test
name: multiple stack objects
arg_out: 0 buffer int[1]   2
arg_out: 1 buffer float[1] 2.0
arg_in:  2 int 0

!*/

kernel void test(global int *out_i, global float *out_f, int index) {

	int stack_i[2];
	float stack_f[2];

	stack_i[0] = 2;
	stack_i[1] = 3;
	stack_f[0] = 2.0f;
	stack_f[1] = 3.0f;

	out_i[0] = stack_i[index];
	out_f[0] = stack_f[index];
}
