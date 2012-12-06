/*!
[config]
name: Int arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 int 1
arg_in:  2 int 2
arg_out: 0 buffer int[1] 3

[test]
name: pos+neg
kernel_name: add
arg_in:  1 int 3
arg_in:  2 int -4
arg_out: 0 buffer int[1] -1

[test]
name: neg+pos
kernel_name: add
arg_in:  1 int -2
arg_in:  2 int 5
arg_out: 0 buffer int[1] 3

[test]
name: neg+neg
kernel_name: add
arg_in:  1 int -2
arg_in:  2 int -3
arg_out: 0 buffer int[1] -5

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 int 1
arg_in:  2 int 2
arg_out: 0 buffer int[1] -1

[test]
name: pos-neg
kernel_name: sub
arg_in:  1 int 3
arg_in:  2 int -4
arg_out: 0 buffer int[1] 7

[test]
name: neg-pos
kernel_name: sub
arg_in:  1 int -2
arg_in:  2 int 5
arg_out: 0 buffer int[1] -7

[test]
name: neg-neg
kernel_name: sub
arg_in:  1 int -2
arg_in:  2 int -3
arg_out: 0 buffer int[1] 1

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 int 3
arg_in:  2 int 4
arg_out: 0 buffer int[1] 12

[test]
name: pos*neg
kernel_name: mul
arg_in:  1 int 3
arg_in:  2 int -3
arg_out: 0 buffer int[1] -9

[test]
name: neg*pos
kernel_name: mul
arg_in:  1 int -2
arg_in:  2 int 5
arg_out: 0 buffer int[1] -10

[test]
name: neg*neg
kernel_name: mul
arg_in:  1 int -2
arg_in:  2 int -3
arg_out: 0 buffer int[1] 6

[test]
name: 0*num
kernel_name: mul
arg_in:  1 int 0
arg_in:  2 int -3
arg_out: 0 buffer int[1] 0

## Division ##

[test]
name: pos div pos
kernel_name: div
arg_in:  1 int 8
arg_in:  2 int 4
arg_out: 0 buffer int[1] 2

[test]
name: pos div pos (remainder)
kernel_name: div
arg_in:  1 int 7
arg_in:  2 int 4
arg_out: 0 buffer int[1] 1

[test]
name: pos div neg
kernel_name: div
arg_in:  1 int 8
arg_in:  2 int -4
arg_out: 0 buffer int[1] -2

[test]
name: pos div neg (remainder)
kernel_name: div
arg_in:  1 int 8
arg_in:  2 int -3
arg_out: 0 buffer int[1] -2

[test]
name: neg div pos
kernel_name: div
arg_in:  1 int -20
arg_in:  2 int 5
arg_out: 0 buffer int[1] -4

[test]
name: neg div pos (remainder)
kernel_name: div
arg_in:  1 int -2
arg_in:  2 int 5
arg_out: 0 buffer int[1] 0

[test]
name: neg div neg
kernel_name: div
arg_in:  1 int -9
arg_in:  2 int -3
arg_out: 0 buffer int[1] 3

[test]
name: neg div neg (remainder)
kernel_name: div
arg_in:  1 int -8
arg_in:  2 int -3
arg_out: 0 buffer int[1] 2

[test]
name: 0 div num
kernel_name: div
arg_in:  1 int 0
arg_in:  2 int -3
arg_out: 0 buffer int[1] 0

## Modulo ##

[test]
name: pos%pos=0
kernel_name: mod
arg_in:  1 int 8
arg_in:  2 int 4
arg_out: 0 buffer int[1] 0

[test]
name: pos%pos=pos
kernel_name: mod
arg_in:  1 int 8
arg_in:  2 int 5
arg_out: 0 buffer int[1] 3

[test]
name: neg%pos=0
kernel_name: mod
arg_in:  1 int -30
arg_in:  2 int 15
arg_out: 0 buffer int[1] 0

[test]
name: neg%pos=neg
kernel_name: mod
arg_in:  1 int -18
arg_in:  2 int 5
arg_out: 0 buffer int[1] -3

[test]
name: pos%neg=0
kernel_name: mod
arg_in:  1 int 12
arg_in:  2 int -4
arg_out: 0 buffer int[1] 0

[test]
name: pos%neg=pos
kernel_name: mod
arg_in:  1 int 16
arg_in:  2 int -3
arg_out: 0 buffer int[1] 1

[test]
name: small_pos%big_pos
kernel_name: mod
arg_in:  1 int 5
arg_in:  2 int 345234
arg_out: 0 buffer int[1] 5

[test]
name: max_int%num
kernel_name: mod
arg_in:  1 int 2147483647
arg_in:  2 int 12345
arg_out: 0 buffer int[1] 9172

[test]
name: min_int%num
kernel_name: mod
arg_in:  1 int -2147483648
arg_in:  2 int 476
arg_out: 0 buffer int[1] -128

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 int 345
arg_out: 0 buffer int[1] 345

[test]
name: +neg
kernel_name: plus
arg_in:  1 int -455
arg_out: 0 buffer int[1] -455

## Unary minus ##

[test]
name: -pos
kernel_name: minus
arg_in:  1 int 345
arg_out: 0 buffer int[1] -345

[test]
name: -neg
kernel_name: minus
arg_in:  1 int -455
arg_out: 0 buffer int[1] 455

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 int 34
arg_out: 0 buffer int[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 int -22
arg_out: 0 buffer int[1] -21

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 int -34
arg_out: 0 buffer int[1] -34

[test]
name: --num
kernel_name: predec
arg_in:  1 int 22
arg_out: 0 buffer int[1] 21

!*/

kernel void add(global int* out, int a, int b) {
	out[0] = a + b;
}

kernel void sub(global int* out, int a, int b) {
	out[0] = a - b;
}

kernel void mul(global int* out, int a, int b) {
	out[0] = a * b;
}

kernel void div(global int* out, int a, int b) {
	out[0] = a / b;
}

kernel void mod(global int* out, int a, int b) {
	out[0] = a % b;
}

kernel void plus(global int* out, int in) {
	out[0] = +in;
}

kernel void minus(global int* out, int in) {
	out[0] = -in;
}

kernel void postinc(global int* out, int in) {
	out[0] = in++;
}

kernel void preinc(global int* out, int in) {
	out[0] = ++in;
}

kernel void postdec(global int* out, int in) {
	out[0] = in--;
}

kernel void predec(global int* out, int in) {
	out[0] = --in;
}
