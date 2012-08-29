/*!

# Boolean values expand to integer, so we use int for results.
# The value true expands to 1 and the value false expands to 0

[config]
name: Int logical operators
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Logical not ##

[test]
name: !true
kernel_name: not
arg_in:  1 int 1
arg_out: 0 buffer int[1] 0

[test]
name: !false
kernel_name: not
arg_in:  1 int 0
arg_out: 0 buffer int[1] 1

[test]
name: !big_num
kernel_name: not
arg_in:  1 int 3452
arg_out: 0 buffer int[1] 0

## Logical and ##

[test]
name: true&&true
kernel_name: and
arg_in:  1 int 1
arg_in:  2 int 1
arg_out: 0 buffer int[1] 1

[test]
name: true&&false
kernel_name: and
arg_in:  1 int 1
arg_in:  2 int 0
arg_out: 0 buffer int[1] 0

[test]
name: false&&false
kernel_name: and
arg_in:  1 int 0
arg_in:  2 int 0
arg_out: 0 buffer int[1] 0

## Logical or ##

[test]
name: true||true
kernel_name: or
arg_in:  1 int 1
arg_in:  2 int 1
arg_out: 0 buffer int[1] 1

[test]
name: true||false
kernel_name: or
arg_in:  1 int 1
arg_in:  2 int 0
arg_out: 0 buffer int[1] 1

[test]
name: false||false
kernel_name: or
arg_in:  1 int 0
arg_in:  2 int 0
arg_out: 0 buffer int[1] 0

!*/

kernel void not(global int* out, int in) {
	out[0] = !in;
}

kernel void and(global int* out, int a, int b) {
	out[0] = a && b;
}

kernel void or(global int* out, int a, int b) {
	out[0] = a || b;
}
