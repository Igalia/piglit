/*!
[config]
name: Ushort arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 ushort 1
arg_in:  2 ushort 2
arg_out: 0 buffer ushort[1] 3

[test]
name: pos+0
kernel_name: add
arg_in:  1 ushort 34
arg_in:  2 ushort 0
arg_out: 0 buffer ushort[1] 34

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 ushort 5
arg_in:  2 ushort 2
arg_out: 0 buffer ushort[1] 3

[test]
name: pos-0
kernel_name: sub
arg_in:  1 ushort 77
arg_in:  2 ushort 0
arg_out: 0 buffer ushort[1] 77

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 ushort 3
arg_in:  2 ushort 4
arg_out: 0 buffer ushort[1] 12

[test]
name: 0*pos
kernel_name: mul
arg_in:  1 ushort 0
arg_in:  2 ushort 3
arg_out: 0 buffer ushort[1] 0

## Division ##

[test]
name: pos div pos
kernel_name: div
arg_in:  1 ushort 8
arg_in:  2 ushort 4
arg_out: 0 buffer ushort[1] 2

[test]
name: pos div pos (remainder)
kernel_name: div
arg_in:  1 ushort 7
arg_in:  2 ushort 4
arg_out: 0 buffer ushort[1] 1

[test]
name: 0 div num
kernel_name: div
arg_in:  1 ushort 0
arg_in:  2 ushort 77
arg_out: 0 buffer ushort[1] 0

## Modulo ##

[test]
name: pos%pos=0
kernel_name: mod
arg_in:  1 ushort 8
arg_in:  2 ushort 4
arg_out: 0 buffer ushort[1] 0

[test]
name: pos mod pos=pos
kernel_name: mod
arg_in:  1 ushort 8
arg_in:  2 ushort 5
arg_out: 0 buffer ushort[1] 3

[test]
name: small_pos mod big_pos
kernel_name: mod
arg_in:  1 ushort 5
arg_in:  2 ushort 32001
arg_out: 0 buffer ushort[1] 5

[test]
name: max_ushort mod num
kernel_name: mod
arg_in:  1 ushort 65535
arg_in:  2 ushort 334
arg_out: 0 buffer ushort[1] 71

[test]
name: min_ushort mod num
kernel_name: mod
arg_in:  1 ushort 0
arg_in:  2 ushort 3453
arg_out: 0 buffer ushort[1] 0

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 ushort 34
arg_out: 0 buffer ushort[1] 34

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 ushort 34
arg_out: 0 buffer ushort[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 ushort 22
arg_out: 0 buffer ushort[1] 23

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 ushort 55
arg_out: 0 buffer ushort[1] 55

[test]
name: --num
kernel_name: predec
arg_in:  1 ushort 22
arg_out: 0 buffer ushort[1] 21

!*/

kernel void add(global ushort* out, ushort a, ushort b) {
	out[0] = a + b;
}

kernel void sub(global ushort* out, ushort a, ushort b) {
	out[0] = a - b;
}

kernel void mul(global ushort* out, ushort a, ushort b) {
	out[0] = a * b;
}

kernel void div(global ushort* out, ushort a, ushort b) {
	out[0] = a / b;
}

kernel void mod(global ushort* out, ushort a, ushort b) {
	out[0] = a % b;
}

kernel void plus(global ushort* out, ushort in) {
	out[0] = +in;
}

kernel void postinc(global ushort* out, ushort in) {
	out[0] = in++;
}

kernel void preinc(global ushort* out, ushort in) {
	out[0] = ++in;
}

kernel void postdec(global ushort* out, ushort in) {
	out[0] = in--;
}

kernel void predec(global ushort* out, ushort in) {
	out[0] = --in;
}
