/*!
[config]
name: Ulong arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 ulong 1
arg_in:  2 ulong 2
arg_out: 0 buffer ulong[1] 3

[test]
name: pos+0
kernel_name: add
arg_in:  1 ulong 34
arg_in:  2 ulong 0
arg_out: 0 buffer ulong[1] 34

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 ulong 5
arg_in:  2 ulong 2
arg_out: 0 buffer ulong[1] 3

[test]
name: pos-0
kernel_name: sub
arg_in:  1 ulong 77
arg_in:  2 ulong 0
arg_out: 0 buffer ulong[1] 77

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 ulong 3
arg_in:  2 ulong 4
arg_out: 0 buffer ulong[1] 12

[test]
name: 0*pos
kernel_name: mul
arg_in:  1 ulong 0
arg_in:  2 ulong 3
arg_out: 0 buffer ulong[1] 0

## Division ##

[test]
name: pos div pos
kernel_name: div
arg_in:  1 ulong 8
arg_in:  2 ulong 4
arg_out: 0 buffer ulong[1] 2

[test]
name: pos div pos (remainder)
kernel_name: div
arg_in:  1 ulong 7
arg_in:  2 ulong 4
arg_out: 0 buffer ulong[1] 1

[test]
name: 0 div num
kernel_name: div
arg_in:  1 ulong 0
arg_in:  2 ulong 77
arg_out: 0 buffer ulong[1] 0

## Modulo ##

[test]
name: pos%pos=0
kernel_name: mod
arg_in:  1 ulong 8
arg_in:  2 ulong 4
arg_out: 0 buffer ulong[1] 0

[test]
name: pos%pos=pos
kernel_name: mod
arg_in:  1 ulong 8
arg_in:  2 ulong 5
arg_out: 0 buffer ulong[1] 3

[test]
name: small_pos%big_pos
kernel_name: mod
arg_in:  1 ulong 7
arg_in:  2 ulong 3200345334351
arg_out: 0 buffer ulong[1] 7

[test]
name: max_ulong%num
kernel_name: mod
arg_in:  1 ulong 18446744073709551615
arg_in:  2 ulong 7674334534554
arg_out: 0 buffer ulong[1] 7547678378247

[test]
name: min_ulong%num
kernel_name: mod
arg_in:  1 ulong 0
arg_in:  2 ulong 342346853345563
arg_out: 0 buffer ulong[1] 0

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 ulong 34
arg_out: 0 buffer ulong[1] 34

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 ulong 34
arg_out: 0 buffer ulong[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 ulong 22
arg_out: 0 buffer ulong[1] 23

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 ulong 55
arg_out: 0 buffer ulong[1] 55

[test]
name: --num
kernel_name: predec
arg_in:  1 ulong 22
arg_out: 0 buffer ulong[1] 21

!*/

kernel void add(global ulong* out, ulong a, ulong b) {
	out[0] = a + b;
}

kernel void sub(global ulong* out, ulong a, ulong b) {
	out[0] = a - b;
}

kernel void mul(global ulong* out, ulong a, ulong b) {
	out[0] = a * b;
}

kernel void div(global ulong* out, ulong a, ulong b) {
	out[0] = a / b;
}

kernel void mod(global ulong* out, ulong a, ulong b) {
	out[0] = a % b;
}

kernel void plus(global ulong* out, ulong in) {
	out[0] = +in;
}

kernel void postinc(global ulong* out, ulong in) {
	out[0] = in++;
}

kernel void preinc(global ulong* out, ulong in) {
	out[0] = ++in;
}

kernel void postdec(global ulong* out, ulong in) {
	out[0] = in--;
}

kernel void predec(global ulong* out, ulong in) {
	out[0] = --in;
}
