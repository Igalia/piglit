/*!
[config]
name: Uchar arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 uchar 1
arg_in:  2 uchar 2
arg_out: 0 buffer uchar[1] 3

[test]
name: pos+0
kernel_name: add
arg_in:  1 uchar 34
arg_in:  2 uchar 0
arg_out: 0 buffer uchar[1] 34

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 uchar 5
arg_in:  2 uchar 2
arg_out: 0 buffer uchar[1] 3

[test]
name: pos-0
kernel_name: sub
arg_in:  1 uchar 77
arg_in:  2 uchar 0
arg_out: 0 buffer uchar[1] 77

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 uchar 3
arg_in:  2 uchar 4
arg_out: 0 buffer uchar[1] 12

[test]
name: 0*pos
kernel_name: mul
arg_in:  1 uchar 0
arg_in:  2 uchar 3
arg_out: 0 buffer uchar[1] 0

## Division ##

[test]
name: pos/pos
kernel_name: div
arg_in:  1 uchar 8
arg_in:  2 uchar 4
arg_out: 0 buffer uchar[1] 2

[test]
name: pos/pos (remainder)
kernel_name: div
arg_in:  1 uchar 7
arg_in:  2 uchar 4
arg_out: 0 buffer uchar[1] 1

[test]
name: 0/num
kernel_name: div
arg_in:  1 uchar 0
arg_in:  2 uchar 77
arg_out: 0 buffer uchar[1] 0

## Modulo ##

[test]
name: pos%pos=0
kernel_name: mod
arg_in:  1 uchar 8
arg_in:  2 uchar 4
arg_out: 0 buffer uchar[1] 0

[test]
name: pos%pos=pos
kernel_name: mod
arg_in:  1 uchar 8
arg_in:  2 uchar 5
arg_out: 0 buffer uchar[1] 3

[test]
name: small_pos%big_pos
kernel_name: mod
arg_in:  1 uchar 5
arg_in:  2 uchar 103
arg_out: 0 buffer uchar[1] 5

[test]
name: max_uchar%num
kernel_name: mod
arg_in:  1 uchar 255
arg_in:  2 uchar 33
arg_out: 0 buffer uchar[1] 24

[test]
name: min_uchar%num
kernel_name: mod
arg_in:  1 uchar 0
arg_in:  2 uchar 46
arg_out: 0 buffer uchar[1] 0

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 uchar 34
arg_out: 0 buffer uchar[1] 34

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 uchar 34
arg_out: 0 buffer uchar[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 uchar 22
arg_out: 0 buffer uchar[1] 23

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 uchar 55
arg_out: 0 buffer uchar[1] 55

[test]
name: --num
kernel_name: predec
arg_in:  1 uchar 22
arg_out: 0 buffer uchar[1] 21

!*/

kernel void add(global uchar* out, uchar a, uchar b) {
	out[0] = a + b;
}

kernel void sub(global uchar* out, uchar a, uchar b) {
	out[0] = a - b;
}

kernel void mul(global uchar* out, uchar a, uchar b) {
	out[0] = a * b;
}

kernel void div(global uchar* out, uchar a, uchar b) {
	out[0] = a / b;
}

kernel void mod(global uchar* out, uchar a, uchar b) {
	out[0] = a % b;
}

kernel void plus(global uchar* out, uchar in) {
	out[0] = +in;
}

kernel void postinc(global uchar* out, uchar in) {
	out[0] = in++;
}

kernel void preinc(global uchar* out, uchar in) {
	out[0] = ++in;
}

kernel void postdec(global uchar* out, uchar in) {
	out[0] = in--;
}

kernel void predec(global uchar* out, uchar in) {
	out[0] = --in;
}
