/*!
[config]
name: Char arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 char 1
arg_in:  2 char 2
arg_out: 0 buffer char[1] 3

[test]
name: pos+neg
kernel_name: add
arg_in:  1 char 3
arg_in:  2 char -4
arg_out: 0 buffer char[1] -1

[test]
name: neg+pos
kernel_name: add
arg_in:  1 char -2
arg_in:  2 char 5
arg_out: 0 buffer char[1] 3

[test]
name: neg+neg
kernel_name: add
arg_in:  1 char -2
arg_in:  2 char -3
arg_out: 0 buffer char[1] -5

## Subtraction ##

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 char 1
arg_in:  2 char 2
arg_out: 0 buffer char[1] -1

[test]
name: pos-neg
kernel_name: sub
arg_in:  1 char 3
arg_in:  2 char -4
arg_out: 0 buffer char[1] 7

[test]
name: neg-pos
kernel_name: sub
arg_in:  1 char -2
arg_in:  2 char 5
arg_out: 0 buffer char[1] -7

[test]
name: neg-neg
kernel_name: sub
arg_in:  1 char -2
arg_in:  2 char -3
arg_out: 0 buffer char[1] 1

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 char 3
arg_in:  2 char 4
arg_out: 0 buffer char[1] 12

[test]
name: pos*neg
kernel_name: mul
arg_in:  1 char 3
arg_in:  2 char -3
arg_out: 0 buffer char[1] -9

[test]
name: neg*pos
kernel_name: mul
arg_in:  1 char -2
arg_in:  2 char 5
arg_out: 0 buffer char[1] -10

[test]
name: neg*neg
kernel_name: mul
arg_in:  1 char -2
arg_in:  2 char -3
arg_out: 0 buffer char[1] 6

[test]
name: 0*num
kernel_name: mul
arg_in:  1 char 0
arg_in:  2 char -3
arg_out: 0 buffer char[1] 0

## Division ##

[test]
name: pos div pos
kernel_name: div
arg_in:  1 char 8
arg_in:  2 char 4
arg_out: 0 buffer char[1] 2

[test]
name: pos div pos (remainder)
kernel_name: div
arg_in:  1 char 7
arg_in:  2 char 4
arg_out: 0 buffer char[1] 1

[test]
name: pos div neg
kernel_name: div
arg_in:  1 char 8
arg_in:  2 char -4
arg_out: 0 buffer char[1] -2

[test]
name: pos div neg (remainder)
kernel_name: div
arg_in:  1 char 8
arg_in:  2 char -3
arg_out: 0 buffer char[1] -2

[test]
name: neg div pos
kernel_name: div
arg_in:  1 char -20
arg_in:  2 char 5
arg_out: 0 buffer char[1] -4

[test]
name: neg div pos (remainder)
kernel_name: div
arg_in:  1 char -2
arg_in:  2 char 5
arg_out: 0 buffer char[1] 0

[test]
name: neg div neg
kernel_name: div
arg_in:  1 char -9
arg_in:  2 char -3
arg_out: 0 buffer char[1] 3

[test]
name: neg div neg (remainder)
kernel_name: div
arg_in:  1 char -8
arg_in:  2 char -3
arg_out: 0 buffer char[1] 2

[test]
name: 0 div num
kernel_name: div
arg_in:  1 char 0
arg_in:  2 char -3
arg_out: 0 buffer char[1] 0

## Modulo ##

[test]
name: pos mod pos=0
kernel_name: mod
arg_in:  1 char 8
arg_in:  2 char 4
arg_out: 0 buffer char[1] 0

[test]
name: pos mod pos=pos
kernel_name: mod
arg_in:  1 char 8
arg_in:  2 char 5
arg_out: 0 buffer char[1] 3

[test]
name: neg mod pos=0
kernel_name: mod
arg_in:  1 char -30
arg_in:  2 char 15
arg_out: 0 buffer char[1] 0

[test]
name: neg mod pos=neg
kernel_name: mod
arg_in:  1 char -18
arg_in:  2 char 5
arg_out: 0 buffer char[1] -3

[test]
name: pos mod neg=0
kernel_name: mod
arg_in:  1 char 12
arg_in:  2 char -4
arg_out: 0 buffer char[1] 0

[test]
name: pos mod neg=pos
kernel_name: mod
arg_in:  1 char 16
arg_in:  2 char -3
arg_out: 0 buffer char[1] 1

[test]
name: small_pos mod big_pos
kernel_name: mod
arg_in:  1 char 5
arg_in:  2 char 111
arg_out: 0 buffer char[1] 5

[test]
name: max_char mod num
kernel_name: mod
arg_in:  1 char 127
arg_in:  2 char 33
arg_out: 0 buffer char[1] 28

[test]
name: min_char mod num
kernel_name: mod
arg_in:  1 char -128
arg_in:  2 char 46
arg_out: 0 buffer char[1] -36

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 char 34
arg_out: 0 buffer char[1] 34

[test]
name: +neg
kernel_name: plus
arg_in:  1 char -45
arg_out: 0 buffer char[1] -45

## Unary minus ##

[test]
name: -pos
kernel_name: minus
arg_in:  1 char 35
arg_out: 0 buffer char[1] -35

[test]
name: -neg
kernel_name: minus
arg_in:  1 char -55
arg_out: 0 buffer char[1] 55

## Increment ##

[test]
name: num++
kernel_name: postinc
arg_in:  1 char 34
arg_out: 0 buffer char[1] 34

[test]
name: ++num
kernel_name: preinc
arg_in:  1 char -22
arg_out: 0 buffer char[1] -21

## Decrement ##

[test]
name: num--
kernel_name: postdec
arg_in:  1 char -34
arg_out: 0 buffer char[1] -34

[test]
name: --num
kernel_name: predec
arg_in:  1 char 22
arg_out: 0 buffer char[1] 21

!*/

kernel void add(global char* out, char a, char b) {
	out[0] = a + b;
}

kernel void sub(global char* out, char a, char b) {
	out[0] = a - b;
}

kernel void mul(global char* out, char a, char b) {
	out[0] = a * b;
}

kernel void div(global char* out, char a, char b) {
	out[0] = a / b;
}

kernel void mod(global char* out, char a, char b) {
	out[0] = a % b;
}

kernel void plus(global char* out, char in) {
	out[0] = +in;
}

kernel void minus(global char* out, char in) {
	out[0] = -in;
}

kernel void postinc(global char* out, char in) {
	out[0] = in++;
}

kernel void preinc(global char* out, char in) {
	out[0] = ++in;
}

kernel void postdec(global char* out, char in) {
	out[0] = in--;
}

kernel void predec(global char* out, char in) {
	out[0] = --in;
}
