/*!

# OpenCL has declared all C99 keywords as reserved words.
# At least one CL run-time incorrectly treats ISO646 Alternative Operator
# Spellings defined in iso646.h as reserved (these are also C++ keywords).

[config]
name: Reserved Words

dimensions: 1
global_size: 1 0 0

[test]
name: and
kernel_name: and
arg_in:  1 int 1
arg_out: 0 buffer int[1] 1

[test]
name: and_eq
kernel_name: and_eq
arg_in:  1 int 1
arg_out: 0 buffer int[1] 2

[test]
name: bitand
kernel_name: bitand
arg_in:  1 int 1
arg_out: 0 buffer int[1] 3

[test]
name: bitor
kernel_name: bitor
arg_in:  1 int 1
arg_out: 0 buffer int[1] 4

[test]
name: compl
kernel_name: compl
arg_in:  1 int 1
arg_out: 0 buffer int[1] 5

[test]
name: not
kernel_name: not
arg_in:  1 int 1
arg_out: 0 buffer int[1] 6

[test]
name: not_eq
kernel_name: not_eq
arg_in:  1 int 1
arg_out: 0 buffer int[1] 7

[test]
name: or
kernel_name: or
arg_in:  1 int 1
arg_out: 0 buffer int[1] 8

[test]
name: or_eq
kernel_name: or_eq
arg_in:  1 int 1
arg_out: 0 buffer int[1] 9

[test]
name: xor
kernel_name: xor
arg_in:  1 int 1
arg_out: 0 buffer int[1] 10

[test]
name: xor_eq
kernel_name: xor_eq
arg_in:  1 int 1
arg_out: 0 buffer int[1] 11

!*/

kernel void and(global int* out, int in){
    out[0] = in * 1;
}

kernel void and_eq(global int* out, int in){
    out[0] = in * 2;
}

kernel void bitand(global int* out, int in){
    out[0] = in * 3;
}

kernel void bitor(global int* out, int in){
    out[0] = in * 4;
}

kernel void compl(global int* out, int in){
    out[0] = in * 5;
}

kernel void not(global int* out, int in){
    out[0] = in * 6;
}

kernel void not_eq(global int* out, int in){
    out[0] = in * 7;
}

kernel void or(global int* out, int in){
    out[0] = in * 8;
}

kernel void or_eq(global int* out, int in){
    out[0] = in * 9;
}

kernel void xor(global int* out, int in){
    out[0] = in * 10;
}

kernel void xor_eq(global int* out, int in){
    out[0] = in * 11;
}
