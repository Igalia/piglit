/*!
[config]
name: Int bitwise
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Bitwise and ##

[test]
name: num&0
kernel_name: and
arg_in:  1 int 3456732
arg_in:  2 int 0
arg_out: 0 buffer int[1] 0

[test]
name: num&num
kernel_name: and
arg_in:  1 uint 0XFFFFFFFF
arg_in:  2 uint 0XAAAAAAAA
arg_out: 0 buffer uint[1] 0XAAAAAAAA

## Bitwise or ##

[test]
name: num|0
kernel_name: or
arg_in:  1 int 3456732
arg_in:  2 int 0
arg_out: 0 buffer int[1] 3456732

[test]
name: num|num
kernel_name: or
arg_in:  1 uint 0X55555555
arg_in:  2 uint 0XAAAAAAAA
arg_out: 0 buffer uint[1] 0XFFFFFFFF

## Bitwise xor ##

[test]
name: num^0
kernel_name: xor
arg_in:  1 int 3456732
arg_in:  2 int 0
arg_out: 0 buffer int[1] 3456732

[test]
name: num^num
kernel_name: xor
arg_in:  1 uint 0XFFFFFFFF
arg_in:  2 uint 0XAAAAAAAA
arg_out: 0 buffer uint[1] 0X55555555

## Bitwise not ##

[test]
name: ~0
kernel_name: not
arg_in:  1 uint 0X0
arg_out: 0 buffer uint[1] 0XFFFFFFFF

[test]
name: ~num
kernel_name: not
arg_in:  1 uint 0X99999999
arg_out: 0 buffer uint[1] 0X66666666

## Bitwise left shift ##

[test]
name: num<<0
kernel_name: ls
arg_in:  1 int 345343
arg_in:  2 int 0
arg_out: 0 buffer uint[1] 345343

[test]
name: num<<num
kernel_name: ls
arg_in:  1 uint 0XFFFFFFFF
arg_in:  2 int 15
arg_out: 0 buffer uint[1] 0XFFFF8000

## Bitwise right shift ##

[test]
name: num>>0
kernel_name: rs
arg_in:  1 int 345343
arg_in:  2 int 0
arg_out: 0 buffer uint[1] 345343

[test]
name: pos>>num
kernel_name: rs
arg_in:  1 uint 0X7FFFFFFF
arg_in:  2 int 14
arg_out: 0 buffer uint[1] 0X0001FFFF

[test]
name: neg>>num
kernel_name: rs
arg_in:  1 uint 0XFFFFFFFF
arg_in:  2 int 15
arg_out: 0 buffer uint[1] 0XFFFFFFFF

!*/

kernel void and(global int* out, int a, int b) {
	out[0] = a&b;
}

kernel void or(global int* out, int a, int b) {
	out[0] = a|b;
}

kernel void xor(global int* out, int a, int b) {
	out[0] = a^b;
}

kernel void not(global int* out, int in) {
	out[0] = ~in;
}

kernel void ls(global int* out, int a, int b) {
	out[0] = a<<b;
}

kernel void rs(global int* out, int a, int b) {
	out[0] = a>>b;
}
