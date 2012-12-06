/*!
[config]
name: Short arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 short 1
arg_in:  2 short 2
arg_out: 0 buffer short[1] 3

[test]
name: pos+neg
kernel_name: add
arg_in:  1 short 3
arg_in:  2 short -4
arg_out: 0 buffer short[1] -1

[test]
name: neg+pos
kernel_name: add
arg_in:  1 short -2
arg_in:  2 short 5
arg_out: 0 buffer short[1] 3

[test]
name: neg+neg
kernel_name: add
arg_in:  1 short -2
arg_in:  2 short -3
arg_out: 0 buffer short[1] -5

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 short 1
arg_in:  2 short 2
arg_out: 0 buffer short[1] -1

[test]
name: pos-neg
kernel_name: sub
arg_in:  1 short 3
arg_in:  2 short -4
arg_out: 0 buffer short[1] 7

[test]
name: neg-pos
kernel_name: sub
arg_in:  1 short -2
arg_in:  2 short 5
arg_out: 0 buffer short[1] -7

[test]
name: neg-neg
kernel_name: sub
arg_in:  1 short -2
arg_in:  2 short -3
arg_out: 0 buffer short[1] 1

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 short 3
arg_in:  2 short 4
arg_out: 0 buffer short[1] 12

[test]
name: pos*neg
kernel_name: mul
arg_in:  1 short 3
arg_in:  2 short -3
arg_out: 0 buffer short[1] -9

[test]
name: neg*pos
kernel_name: mul
arg_in:  1 short -2
arg_in:  2 short 5
arg_out: 0 buffer short[1] -10

[test]
name: neg*neg
kernel_name: mul
arg_in:  1 short -2
arg_in:  2 short -3
arg_out: 0 buffer short[1] 6

[test]
name: 0*num
kernel_name: mul
arg_in:  1 short 0
arg_in:  2 short -3
arg_out: 0 buffer short[1] 0

## Division ##

[test]
name: pos div pos
kernel_name: div
arg_in:  1 short 8
arg_in:  2 short 4
arg_out: 0 buffer short[1] 2

[test]
name: pos div pos (remainder)
kernel_name: div
arg_in:  1 short 7
arg_in:  2 short 4
arg_out: 0 buffer short[1] 1

[test]
name: pos div neg
kernel_name: div
arg_in:  1 short 8
arg_in:  2 short -4
arg_out: 0 buffer short[1] -2

[test]
name: pos div neg (remainder)
kernel_name: div
arg_in:  1 short 8
arg_in:  2 short -3
arg_out: 0 buffer short[1] -2

[test]
name: neg div pos
kernel_name: div
arg_in:  1 short -20
arg_in:  2 short 5
arg_out: 0 buffer short[1] -4

[test]
name: neg div pos (remainder)
kernel_name: div
arg_in:  1 short -2
arg_in:  2 short 5
arg_out: 0 buffer short[1] 0

[test]
name: neg div neg
kernel_name: div
arg_in:  1 short -9
arg_in:  2 short -3
arg_out: 0 buffer short[1] 3

[test]
name: neg div neg (remainder)
kernel_name: div
arg_in:  1 short -8
arg_in:  2 short -3
arg_out: 0 buffer short[1] 2

[test]
name: 0 div num
kernel_name: div
arg_in:  1 short 0
arg_in:  2 short -3
arg_out: 0 buffer short[1] 0

## Modulo ##

[test]
name: pos%pos=0
kernel_name: mod
arg_in:  1 short 8
arg_in:  2 short 4
arg_out: 0 buffer short[1] 0

[test]
name: pos%pos=pos
kernel_name: mod
arg_in:  1 short 8
arg_in:  2 short 5
arg_out: 0 buffer short[1] 3

[test]
name: neg%pos=0
kernel_name: mod
arg_in:  1 short -30
arg_in:  2 short 15
arg_out: 0 buffer short[1] 0

[test]
name: neg%pos=neg
kernel_name: mod
arg_in:  1 short -18
arg_in:  2 short 5
arg_out: 0 buffer short[1] -3

[test]
name: pos%neg=0
kernel_name: mod
arg_in:  1 short 12
arg_in:  2 short -4
arg_out: 0 buffer short[1] 0

[test]
name: pos%neg=pos
kernel_name: mod
arg_in:  1 short 16
arg_in:  2 short -3
arg_out: 0 buffer short[1] 1

[test]
name: small_pos%big_pos
kernel_name: mod
arg_in:  1 short 5
arg_in:  2 short 345
arg_out: 0 buffer short[1] 5

[test]
name: max_short%num
kernel_name: mod
arg_in:  1 short 32767
arg_in:  2 short 1234
arg_out: 0 buffer short[1] 683

[test]
name: min_short%num
kernel_name: mod
arg_in:  1 short -32768
arg_in:  2 short 475
arg_out: 0 buffer short[1] -468

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 short 345
arg_out: 0 buffer short[1] 345

[test]
name: +neg
kernel_name: plus
arg_in:  1 short -455
arg_out: 0 buffer short[1] -455

## Unary minus ##

[test]
name: -pos
kernel_name: minus
arg_in:  1 short 345
arg_out: 0 buffer short[1] -345

[test]
name: -neg
kernel_name: minus
arg_in:  1 short -455
arg_out: 0 buffer short[1] 455

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 short 34
arg_out: 0 buffer short[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 short -22
arg_out: 0 buffer short[1] -21

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 short -34
arg_out: 0 buffer short[1] -34

[test]
name: --num
kernel_name: predec
arg_in:  1 short 22
arg_out: 0 buffer short[1] 21

!*/

kernel void add(global short* out, short a, short b) {
	out[0] = a + b;
}

kernel void sub(global short* out, short a, short b) {
	out[0] = a - b;
}

kernel void mul(global short* out, short a, short b) {
	out[0] = a * b;
}

kernel void div(global short* out, short a, short b) {
	out[0] = a / b;
}

kernel void mod(global short* out, short a, short b) {
	out[0] = a % b;
}

kernel void plus(global short* out, short in) {
	out[0] = +in;
}

kernel void minus(global short* out, short in) {
	out[0] = -in;
}

kernel void postinc(global short* out, short in) {
	out[0] = in++;
}

kernel void preinc(global short* out, short in) {
	out[0] = ++in;
}

kernel void postdec(global short* out, short in) {
	out[0] = in--;
}

kernel void predec(global short* out, short in) {
	out[0] = --in;
}
