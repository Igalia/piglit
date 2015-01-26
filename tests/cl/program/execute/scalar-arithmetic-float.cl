/*!
[config]
name: Float arithmetic
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

## Addition ##

[test]
name: pos+pos
kernel_name: add
arg_in:  1 float 1.0
arg_in:  2 float 2.5
arg_out: 0 buffer float[1] 3.5

[test]
name: pos+neg
kernel_name: add
arg_in:  1 float 3.25
arg_in:  2 float -4.5
arg_out: 0 buffer float[1] -1.25

[test]
name: neg+pos
kernel_name: add
arg_in:  1 float -2.125
arg_in:  2 float 5.625
arg_out: 0 buffer float[1] 3.5

[test]
name: neg+neg
kernel_name: add
arg_in:  1 float -2.25
arg_in:  2 float -3.125
arg_out: 0 buffer float[1] -5.375

[test]
name: inf+inf
kernel_name: add
arg_in:  1 float inf
arg_in:  2 float inf
arg_out: 0 buffer float[1] inf

[test]
name: -inf+-inf
kernel_name: add
arg_in:  1 float -inf
arg_in:  2 float -inf
arg_out: 0 buffer float[1] -inf

[test]
name: pos-pos
kernel_name: sub
arg_in:  1 float 1.5
arg_in:  2 float 2.5
arg_out: 0 buffer float[1] -1

[test]
name: pos-neg
kernel_name: sub
arg_in:  1 float 3.5
arg_in:  2 float -4
arg_out: 0 buffer float[1] 7.5

[test]
name: neg-pos
kernel_name: sub
arg_in:  1 float -2.75
arg_in:  2 float 5.25
arg_out: 0 buffer float[1] -8

[test]
name: neg-neg
kernel_name: sub
arg_in:  1 float -2.5
arg_in:  2 float -3.75
arg_out: 0 buffer float[1] 1.25

[test]
name: 0-inf
kernel_name: sub
arg_in:  1 float 0
arg_in:  2 float inf
arg_out: 0 buffer float[1] -inf

[test]
name: 0--inf
kernel_name: sub
arg_in:  1 float 0
arg_in:  2 float -inf
arg_out: 0 buffer float[1] inf

## Multiplication ##

[test]
name: pos*pos
kernel_name: mul
arg_in:  1 float 3.5
arg_in:  2 float 4.5
arg_out: 0 buffer float[1] 15.75

[test]
name: pos*neg
kernel_name: mul
arg_in:  1 float 10
arg_in:  2 float -3.125
arg_out: 0 buffer float[1] -31.25

[test]
name: neg*pos
kernel_name: mul
arg_in:  1 float -2.25
arg_in:  2 float 5.5
arg_out: 0 buffer float[1] -12.375

[test]
name: neg*neg
kernel_name: mul
arg_in:  1 float -2
arg_in:  2 float -3.5
arg_out: 0 buffer float[1] 7

[test]
name: 0*pos
kernel_name: mul
arg_in:  1 float 0
arg_in:  2 float 3.17
arg_out: 0 buffer float[1] 0

[test]
name: 0*neg
kernel_name: mul
arg_in:  1 float 0
arg_in:  2 float -4.536
arg_out: 0 buffer float[1] -0

[test]
name: 0*inf
kernel_name: mul
arg_in:  1 float 0
arg_in:  2 float inf
arg_out: 0 buffer float[1] nan

[test]
name: inf*inf
kernel_name: mul
arg_in:  1 float inf
arg_in:  2 float inf
arg_out: 0 buffer float[1] inf

[test]
name: inf*-inf
kernel_name: mul
arg_in:  1 float inf
arg_in:  2 float -inf
arg_out: 0 buffer float[1] -inf

[test]
name: -inf*-inf
kernel_name: mul
arg_in:  1 float -inf
arg_in:  2 float -inf
arg_out: 0 buffer float[1] inf

[test]
name: num*nan
kernel_name: mul
arg_in:  1 float 234.5
arg_in:  2 float nan
arg_out: 0 buffer float[1] nan

## Division ##

[test]
name: pos div pos
kernel_name: div
arg_in:  1 float 8.5
arg_in:  2 float 4.25
arg_out: 0 buffer float[1] 2 tolerance 2

[test]
name: pos div neg
kernel_name: div
arg_in:  1 float 11.25
arg_in:  2 float -3
arg_out: 0 buffer float[1] -3.75 tolerance 2

[test]
name: neg div pos
kernel_name: div
arg_in:  1 float -21
arg_in:  2 float 5.25
arg_out: 0 buffer float[1] -4 tolerance 2

[test]
name: neg div neg
kernel_name: div
arg_in:  1 float -21.25
arg_in:  2 float -5
arg_out: 0 buffer float[1] 4.25 tolerance 2

[test]
name: 0 div pos
kernel_name: div
arg_in:  1 float 0
arg_in:  2 float 3.7
arg_out: 0 buffer float[1] 0 tolerance 2

[test]
name: 0 div neg
kernel_name: div
arg_in:  1 float 0
arg_in:  2 float -3.7
arg_out: 0 buffer float[1] -0 tolerance 2

[test]
name: num div 0
kernel_name: div
arg_in:  1 float 45.25
arg_in:  2 float 0
arg_out: 0 buffer float[1] inf

[test]
name: -num div 0
kernel_name: div
arg_in:  1 float -45.25
arg_in:  2 float 0
arg_out: 0 buffer float[1] -inf

[test]
name: 0 div inf
kernel_name: div
arg_in:  1 float 0
arg_in:  2 float inf
arg_out: 0 buffer float[1] 0 tolerance 2

[test]
name: inf div 0
kernel_name: div
arg_in:  1 float inf
arg_in:  2 float 0
arg_out: 0 buffer float[1] inf

[test]
name: inf div inf
kernel_name: div
arg_in:  1 float inf
arg_in:  2 float inf
arg_out: 0 buffer float[1] nan

[test]
name: num div nan
kernel_name: div
arg_in:  1 float 234.5
arg_in:  2 float nan
arg_out: 0 buffer float[1] nan

[test]
name: nan div 0
kernel_name: div
arg_in:  1 float nan
arg_in:  2 float 0
arg_out: 0 buffer float[1] nan

## Unary plus ##

[test]
name: +pos
kernel_name: plus
arg_in:  1 float 345.5
arg_out: 0 buffer float[1] 345.5

[test]
name: +neg
kernel_name: plus
arg_in:  1 float -455.75
arg_out: 0 buffer float[1] -455.75

[test]
name: +inf
kernel_name: plus
arg_in:  1 float inf
arg_out: 0 buffer float[1] inf

[test]
name: +-inf
kernel_name: plus
arg_in:  1 float -inf
arg_out: 0 buffer float[1] -inf

## Unary minus ##

[test]
name: -pos
kernel_name: minus
arg_in:  1 float 345.25
arg_out: 0 buffer float[1] -345.25

[test]
name: -neg
kernel_name: minus
arg_in:  1 float -455.125
arg_out: 0 buffer float[1] 455.125

[test]
name: -inf
kernel_name: minus
arg_in:  1 float inf
arg_out: 0 buffer float[1] -inf

[test]
name: --inf
kernel_name: minus
arg_in:  1 float -inf
arg_out: 0 buffer float[1] inf

!*/

kernel void add(global float* out, float a, float b) {
	out[0] = a + b;
}

kernel void sub(global float* out, float a, float b) {
	out[0] = a - b;
}

kernel void mul(global float* out, float a, float b) {
	out[0] = a * b;
}

kernel void div(global float* out, float a, float b) {
	out[0] = a / b;
}

kernel void plus(global float* out, float in) {
	out[0] = +in;
}

kernel void minus(global float* out, float in) {
	out[0] = -in;
}
