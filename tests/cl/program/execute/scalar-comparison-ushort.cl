/*!

# Boolean values expand to integer, so we use int for results.
# The value true expands to 1 and the value false expands to 0

[config]
name: Ushort comparison
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Equality ##

[test]
name: num==num
kernel_name: eq
arg_in:  1 ushort 2
arg_in:  2 ushort 2
arg_out: 0 buffer int[1] 1

[test]
name: num1==num2
kernel_name: eq
arg_in:  1 ushort 2
arg_in:  2 ushort 7
arg_out: 0 buffer int[1] 0

## Inequality ##

[test]
name: num!=num
kernel_name: neq
arg_in:  1 ushort 2
arg_in:  2 ushort 2
arg_out: 0 buffer int[1] 0

[test]
name: num1!=num2
kernel_name: neq
arg_in:  1 ushort 2
arg_in:  2 ushort 7
arg_out: 0 buffer int[1] 1

## Greater than ##

[test]
name: num>num
kernel_name: gt
arg_in:  1 ushort 5
arg_in:  2 ushort 5
arg_out: 0 buffer int[1] 0

[test]
name: big_num>small_num
kernel_name: gt
arg_in:  1 ushort 9
arg_in:  2 ushort 7
arg_out: 0 buffer int[1] 1

[test]
name: small_num>big_num
kernel_name: gt
arg_in:  1 ushort 3
arg_in:  2 ushort 5
arg_out: 0 buffer int[1] 0

## Greater than or Equal to ##

[test]
name: num>=num
kernel_name: gte
arg_in:  1 ushort 5
arg_in:  2 ushort 5
arg_out: 0 buffer int[1] 1

[test]
name: big_num>=small_num
kernel_name: gte
arg_in:  1 ushort 9
arg_in:  2 ushort 7
arg_out: 0 buffer int[1] 1

[test]
name: small_num>=big_num
kernel_name: gte
arg_in:  1 ushort 3
arg_in:  2 ushort 4
arg_out: 0 buffer int[1] 0

## Less than ##

[test]
name: num<num
kernel_name: lt
arg_in:  1 ushort 5
arg_in:  2 ushort 5
arg_out: 0 buffer int[1] 0

[test]
name: big_num<small_num
kernel_name: lt
arg_in:  1 ushort 9
arg_in:  2 ushort 7
arg_out: 0 buffer int[1] 0

[test]
name: small_num<big_num
kernel_name: lt
arg_in:  1 ushort 3
arg_in:  2 ushort 4
arg_out: 0 buffer int[1] 1

## Less than or Equal to##

[test]
name: num<=num
kernel_name: lte
arg_in:  1 ushort 5
arg_in:  2 ushort 5
arg_out: 0 buffer int[1] 1

[test]
name: big_num<=small_num
kernel_name: lte
arg_in:  1 ushort 9
arg_in:  2 ushort 7
arg_out: 0 buffer int[1] 0

[test]
name: small_num<=big_num
kernel_name: lte
arg_in:  1 ushort 3
arg_in:  2 ushort 4
arg_out: 0 buffer int[1] 1

!*/

kernel void eq(global int* out, ushort a, ushort b) {
	out[0] = a == b;
}

kernel void neq(global int* out, ushort a, ushort b) {
	out[0] = a != b;
}

kernel void gt(global int* out, ushort a, ushort b) {
	out[0] = a > b;
}

kernel void gte(global int* out, ushort a, ushort b) {
	out[0] = a >= b;
}

kernel void lt(global int* out, ushort a, ushort b) {
	out[0] = a < b;
}

kernel void lte(global int* out, ushort a, ushort b) {
	out[0] = a <= b;
}
