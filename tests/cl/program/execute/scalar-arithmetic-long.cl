/*!
[config]
name: Long arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 long 1
arg_in:  2 long 2
arg_out: 0 buffer long[1] 3

[test]
name: pos+neg
kernel_name: add
arg_in:  1 long 3
arg_in:  2 long -4
arg_out: 0 buffer long[1] -1

[test]
name: neg+pos
kernel_name: add
arg_in:  1 long -2
arg_in:  2 long 5
arg_out: 0 buffer long[1] 3

[test]
name: neg+neg
kernel_name: add
arg_in:  1 long -2
arg_in:  2 long -3
arg_out: 0 buffer long[1] -5

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 long 1
arg_in:  2 long 2
arg_out: 0 buffer long[1] -1

[test]
name: pos-neg
kernel_name: sub
arg_in:  1 long 3
arg_in:  2 long -4
arg_out: 0 buffer long[1] 7

[test]
name: neg-pos
kernel_name: sub
arg_in:  1 long -2
arg_in:  2 long 5
arg_out: 0 buffer long[1] -7

[test]
name: neg-neg
kernel_name: sub
arg_in:  1 long -2
arg_in:  2 long -3
arg_out: 0 buffer long[1] 1

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 long 3
arg_in:  2 long 4
arg_out: 0 buffer long[1] 12

[test]
name: pos*neg
kernel_name: mul
arg_in:  1 long 3
arg_in:  2 long -3
arg_out: 0 buffer long[1] -9

[test]
name: neg*pos
kernel_name: mul
arg_in:  1 long -2
arg_in:  2 long 5
arg_out: 0 buffer long[1] -10

[test]
name: neg*neg
kernel_name: mul
arg_in:  1 long -2
arg_in:  2 long -3
arg_out: 0 buffer long[1] 6

[test]
name: 0*num
kernel_name: mul
arg_in:  1 long 0
arg_in:  2 long -3
arg_out: 0 buffer long[1] 0

## Division ##

[test]
name: pos/pos
kernel_name: div
arg_in:  1 long 8
arg_in:  2 long 4
arg_out: 0 buffer long[1] 2

[test]
name: pos/pos (remainder)
kernel_name: div
arg_in:  1 long 7
arg_in:  2 long 4
arg_out: 0 buffer long[1] 1

[test]
name: pos/neg
kernel_name: div
arg_in:  1 long 8
arg_in:  2 long -4
arg_out: 0 buffer long[1] -2

[test]
name: pos/neg (remainder)
kernel_name: div
arg_in:  1 long 8
arg_in:  2 long -3
arg_out: 0 buffer long[1] -2

[test]
name: neg/pos
kernel_name: div
arg_in:  1 long -20
arg_in:  2 long 5
arg_out: 0 buffer long[1] -4

[test]
name: neg/pos (remainder)
kernel_name: div
arg_in:  1 long -2
arg_in:  2 long 5
arg_out: 0 buffer long[1] 0

[test]
name: neg/neg
kernel_name: div
arg_in:  1 long -9
arg_in:  2 long -3
arg_out: 0 buffer long[1] 3

[test]
name: neg/neg (remainder)
kernel_name: div
arg_in:  1 long -8
arg_in:  2 long -3
arg_out: 0 buffer long[1] 2

[test]
name: 0/num
kernel_name: div
arg_in:  1 long 0
arg_in:  2 long -3
arg_out: 0 buffer long[1] 0

## Modulo ##

[test]
name: pos%pos=0
kernel_name: mod
arg_in:  1 long 8
arg_in:  2 long 4
arg_out: 0 buffer long[1] 0

[test]
name: pos%pos=pos
kernel_name: mod
arg_in:  1 long 8
arg_in:  2 long 5
arg_out: 0 buffer long[1] 3

[test]
name: neg%pos=0
kernel_name: mod
arg_in:  1 long -30
arg_in:  2 long 15
arg_out: 0 buffer long[1] 0

[test]
name: neg%pos=neg
kernel_name: mod
arg_in:  1 long -18
arg_in:  2 long 5
arg_out: 0 buffer long[1] -3

[test]
name: pos%neg=0
kernel_name: mod
arg_in:  1 long 12
arg_in:  2 long -4
arg_out: 0 buffer long[1] 0

[test]
name: pos%neg=pos
kernel_name: mod
arg_in:  1 long 16
arg_in:  2 long -3
arg_out: 0 buffer long[1] 1

[test]
name: small_pos%big_pos
kernel_name: mod
arg_in:  1 long 5
arg_in:  2 long 23423425534456
arg_out: 0 buffer long[1] 5

[test]
name: max_long%num
kernel_name: mod
arg_in:  1 long 9223372036854775807
arg_in:  2 long 99327437623
arg_out: 0 buffer long[1] 2198836057

[test]
name: min_long%num
kernel_name: mod
arg_in:  1 long -9223372036854775808
arg_in:  2 long 63452343342252
arg_out: 0 buffer long[1] -2860968367340

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 long 345
arg_out: 0 buffer long[1] 345

[test]
name: +neg
kernel_name: plus
arg_in:  1 long -455
arg_out: 0 buffer long[1] -455

## Unary minus ##

[test]
name: -pos
kernel_name: minus
arg_in:  1 long 345
arg_out: 0 buffer long[1] -345

[test]
name: -neg
kernel_name: minus
arg_in:  1 long -455
arg_out: 0 buffer long[1] 455

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 long 34
arg_out: 0 buffer long[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 long -22
arg_out: 0 buffer long[1] -21

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 long -34
arg_out: 0 buffer long[1] -34

[test]
name: --num
kernel_name: predec
arg_in:  1 long 22
arg_out: 0 buffer long[1] 21

!*/

kernel void add(global long* out, long a, long b) {
	out[0] = a + b;
}

kernel void sub(global long* out, long a, long b) {
	out[0] = a - b;
}

kernel void mul(global long* out, long a, long b) {
	out[0] = a * b;
}

kernel void div(global long* out, long a, long b) {
	out[0] = a / b;
}

kernel void mod(global long* out, long a, long b) {
	out[0] = a % b;
}

kernel void plus(global long* out, long in) {
	out[0] = +in;
}

kernel void minus(global long* out, long in) {
	out[0] = -in;
}

kernel void postinc(global long* out, long in) {
	out[0] = in++;
}

kernel void preinc(global long* out, long in) {
	out[0] = ++in;
}

kernel void postdec(global long* out, long in) {
	out[0] = in--;
}

kernel void predec(global long* out, long in) {
	out[0] = --in;
}
