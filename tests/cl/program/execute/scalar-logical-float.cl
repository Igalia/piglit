/*!

# Boolean values expand to integer, so we use int for results.
# The value true expands to 1 and the value false expands to 0

[config]
name: Float logical operators
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Logical not ##

[test]
name: !num
kernel_name: test_not
arg_in:  1 float 1.5
arg_out: 0 buffer int[1] 0

[test]
name: !0
kernel_name: test_not
arg_in:  1 float 0
arg_out: 0 buffer int[1] 1

[test]
name: !inf
kernel_name: test_not
arg_in:  1 float inf
arg_out: 0 buffer int[1] 0

## Logical and ##

[test]
name: num&&-num
kernel_name: test_and
arg_in:  1 float 34.25
arg_in:  2 float -55.125
arg_out: 0 buffer int[1] 1

[test]
name: num&&0
kernel_name: test_and
arg_in:  1 float 1.5
arg_in:  2 float 0
arg_out: 0 buffer int[1] 0

[test]
name: 0&&0
kernel_name: test_and
arg_in:  1 float 0
arg_in:  2 float 0
arg_out: 0 buffer int[1] 0

## Logical or ##

[test]
name: num||-num
kernel_name: test_or
arg_in:  1 float 14.3
arg_in:  2 float -34.1
arg_out: 0 buffer int[1] 1

[test]
name: num||0
kernel_name: test_or
arg_in:  1 float 45.3
arg_in:  2 float 0
arg_out: 0 buffer int[1] 1

[test]
name: 0||0
kernel_name: test_or
arg_in:  1 float 0
arg_in:  2 float 0
arg_out: 0 buffer int[1] 0

!*/

kernel void test_not(global int* out, float in) {
	out[0] = !in;
}

kernel void test_and(global int* out, float a, float b) {
	out[0] = a && b;
}

kernel void test_or(global int* out, float a, float b) {
	out[0] = a || b;
}
