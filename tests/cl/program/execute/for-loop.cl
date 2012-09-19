/*!
[config]
name: For loop
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

[test]
name: loop ge
kernel_name: loop_ge
arg_in: 1 int 10
arg_out: 0 buffer int[10] 9 8 7 6 5 4 3 2 1 0

[test]
name: loop gt
kernel_name: loop_gt
arg_in: 1 int 10
arg_out: 0 buffer int[10] 9 8 7 6 5 4 3 2 1 0

[test]
name: loop le
kernel_name: loop_le
arg_in: 1 int 10
arg_out: 0 buffer int[10] 0 1 2 3 4 5 6 7 8 9

[test]
name: loop lt
kernel_name: loop_lt
arg_in: 1 int 10
arg_out: 0 buffer int[10] 0 1 2 3 4 5 6 7 8 9

!*/

kernel void loop_ge(global int* out, int iterations) {
	int i;
	int ai = 0;
	for (i = iterations - 1; i >= 0; i--) {
		out[ai] = i;
		ai = ai+1;
	}
}

kernel void loop_gt(global int* out, int iterations) {
	int i;
	int ai = 0;
	for (i = iterations - 1; i > -1; i--) {
		out[ai] = i;
		ai = ai+1;
	}
}

kernel void loop_le(global int* out, int iterations) {
	int i;
	int ai = 0;
	for (i = 0; i <= (iterations - 1); i++) {
		out[ai] = i;
		ai = ai+1;
	}
}

kernel void loop_lt(global int* out, int iterations) {
	int i;
	int ai = 0;
	for (i = 0; i < iterations; i++) {
		out[ai] = i;
		ai = ai+1;
	}
}
