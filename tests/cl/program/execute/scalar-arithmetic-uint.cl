/*!
[config]
name: Uint arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 uint 1
arg_in:  2 uint 2
arg_out: 0 buffer uint[1] 3

[test]
name: pos+0
kernel_name: add
arg_in:  1 uint 34
arg_in:  2 uint 0
arg_out: 0 buffer uint[1] 34

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 uint 5
arg_in:  2 uint 2
arg_out: 0 buffer uint[1] 3

[test]
name: pos-0
kernel_name: sub
arg_in:  1 uint 77
arg_in:  2 uint 0
arg_out: 0 buffer uint[1] 77

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 uint 3
arg_in:  2 uint 4
arg_out: 0 buffer uint[1] 12

[test]
name: 0*pos
kernel_name: mul
arg_in:  1 uint 0
arg_in:  2 uint 3
arg_out: 0 buffer uint[1] 0

## Division ##

[test]
name: pos div pos
kernel_name: div
arg_in:  1 uint 8
arg_in:  2 uint 4
arg_out: 0 buffer uint[1] 2

[test]
name: pos div pos (remainder)
kernel_name: div
arg_in:  1 uint 7
arg_in:  2 uint 4
arg_out: 0 buffer uint[1] 1

[test]
name: 0 div num
kernel_name: div
arg_in:  1 uint 0
arg_in:  2 uint 77
arg_out: 0 buffer uint[1] 0

## Modulo ##

[test]
name: pos%pos=0
kernel_name: mod
arg_in:  1 uint 8
arg_in:  2 uint 4
arg_out: 0 buffer uint[1] 0

[test]
name: pos%pos=pos
kernel_name: mod
arg_in:  1 uint 8
arg_in:  2 uint 5
arg_out: 0 buffer uint[1] 3

[test]
name: small_pos%big_pos
kernel_name: mod
arg_in:  1 uint 6
arg_in:  2 uint 32004351
arg_out: 0 buffer uint[1] 6

[test]
name: max_uint%num
kernel_name: mod
arg_in:  1 uint 4294967295
arg_in:  2 uint 43563
arg_out: 0 buffer uint[1] 3999

[test]
name: min_uint%num
kernel_name: mod
arg_in:  1 uint 0
arg_in:  2 uint 342346853
arg_out: 0 buffer uint[1] 0

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 uint 34
arg_out: 0 buffer uint[1] 34

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 uint 34
arg_out: 0 buffer uint[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 uint 22
arg_out: 0 buffer uint[1] 23

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 uint 55
arg_out: 0 buffer uint[1] 55

[test]
name: --num
kernel_name: predec
arg_in:  1 uint 22
arg_out: 0 buffer uint[1] 21

!*/

kernel void add(global uint* out, uint a, uint b) {
	out[0] = a + b;
}

kernel void sub(global uint* out, uint a, uint b) {
	out[0] = a - b;
}

kernel void mul(global uint* out, uint a, uint b) {
	out[0] = a * b;
}

kernel void div(global uint* out, uint a, uint b) {
	out[0] = a / b;
}

kernel void mod(global uint* out, uint a, uint b) {
	out[0] = a % b;
}

kernel void plus(global uint* out, uint in) {
	out[0] = +in;
}

kernel void postinc(global uint* out, uint in) {
	out[0] = in++;
}

kernel void preinc(global uint* out, uint in) {
	out[0] = ++in;
}

kernel void postdec(global uint* out, uint in) {
	out[0] = in--;
}

kernel void predec(global uint* out, uint in) {
	out[0] = --in;
}
