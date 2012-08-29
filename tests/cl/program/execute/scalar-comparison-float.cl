/*!

# Boolean values expand to integer, so we use int for results.
# The value true expands to 1 and the value false expands to 0

[config]
name: Float comparison
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Equality ##

[test]
name: num==num
kernel_name: eq
arg_in:  1 float 2.5
arg_in:  2 float 2.5
arg_out: 0 buffer int[1] 1

[test]
name: num1==num2
kernel_name: eq
arg_in:  1 float 2.75
arg_in:  2 float 7.125
arg_out: 0 buffer int[1] 0

[test]
name: num==-num
kernel_name: eq
arg_in:  1 float 2.25
arg_in:  2 float -2.25
arg_out: 0 buffer int[1] 0

## Inequality ##

[test]
name: num!=num
kernel_name: neq
arg_in:  1 float 2.5
arg_in:  2 float 2.5
arg_out: 0 buffer int[1] 0

[test]
name: num1!=num2
kernel_name: neq
arg_in:  1 float 2.75
arg_in:  2 float 7.125
arg_out: 0 buffer int[1] 1

[test]
name: num!=-num
kernel_name: neq
arg_in:  1 float 12.75
arg_in:  2 float -12.75
arg_out: 0 buffer int[1] 1

## Greater than ##

[test]
name: num>num
kernel_name: gt
arg_in:  1 float 5.625
arg_in:  2 float 5.625
arg_out: 0 buffer int[1] 0

[test]
name: big_num>small_num
kernel_name: gt
arg_in:  1 float 9.25
arg_in:  2 float 7.5
arg_out: 0 buffer int[1] 1

[test]
name: small_num>big_num
kernel_name: gt
arg_in:  1 float -3.5
arg_in:  2 float 4.5
arg_out: 0 buffer int[1] 0

## Greater than or Equal to ##

[test]
name: num>=num
kernel_name: gte
arg_in:  1 float 5.626
arg_in:  2 float 5.625
arg_out: 0 buffer int[1] 1

[test]
name: big_num>=small_num
kernel_name: gte
arg_in:  1 float 9.25
arg_in:  2 float 7.5
arg_out: 0 buffer int[1] 1

[test]
name: small_num>=big_num
kernel_name: gte
arg_in:  1 float -3.5
arg_in:  2 float 4.5
arg_out: 0 buffer int[1] 0

## Less than ##

[test]
name: num<num
kernel_name: lt
arg_in:  1 float 5.625
arg_in:  2 float 5.625
arg_out: 0 buffer int[1] 0

[test]
name: big_num<small_num
kernel_name: lt
arg_in:  1 float 9.25
arg_in:  2 float 7.5
arg_out: 0 buffer int[1] 0

[test]
name: small_num<big_num
kernel_name: lt
arg_in:  1 float -3.5
arg_in:  2 float 4.5
arg_out: 0 buffer int[1] 1

## Less than or Equal to##

[test]
name: num<=num
kernel_name: lte
arg_in:  1 float 5.625
arg_in:  2 float 5.625
arg_out: 0 buffer int[1] 1

[test]
name: big_num<=small_num
kernel_name: lte
arg_in:  1 float 9.25
arg_in:  2 float 7.5
arg_out: 0 buffer int[1] 0

[test]
name: small_num<=big_num
kernel_name: lte
arg_in:  1 float -3.5
arg_in:  2 float 4.5
arg_out: 0 buffer int[1] 1

!*/

kernel void eq(global int* out, float a, float b) {
	out[0] = a == b;
}

kernel void neq(global int* out, float a, float b) {
	out[0] = a != b;
}

kernel void gt(global int* out, float a, float b) {
	out[0] = a > b;
}

kernel void gte(global int* out, float a, float b) {
	out[0] = a >= b;
}

kernel void lt(global int* out, float a, float b) {
	out[0] = a < b;
}

kernel void lte(global int* out, float a, float b) {
	out[0] = a <= b;
}
