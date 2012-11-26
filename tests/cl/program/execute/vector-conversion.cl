/*!
[config]
name: Vector Conversion tests
clc_version_min: 10

dimensions: 1
global_size: 1 0 0

[test]
name: Convert scalar int to vector int4 via cast
kernel_name: convert_int_to_int4
arg_in:  1 int            5
arg_out: 0 buffer int4[1]  5 5 5 5

[test]
name: Convert char4 to int4
kernel_name: convert_char4_int4
arg_in:  1 char4 0 1 2 3
arg_out: 0 buffer int4[1]  0 1 2 3

[test]
name: Convert int4 to float4
kernel_name: convert_int4_float4
arg_in:  1 int4 0 1 2 3
arg_out: 0 buffer float4[1]  0 1 2 3

[test]
name: Convert int4 to saturated char4
kernel_name: convert_int4_to_char4_sat
arg_in:  1 int4 -100000 1 2 123456
arg_out: 0 buffer char4[1]  -128 1 2 127

[test]
name: Convert float8 to int8, round to zero (trunc)
kernel_name: convert_float8_to_int8_rtz
arg_in:  1 float8 -23.67 -23.50 -23.35 -23.0 23.0 23.35 23.50 23.67
arg_out: 0 buffer int8[1] -23 -23 -23 -23 23 23 23 23

[test]
name: Convert float8 to int8, round to positive infinity (ceil)
kernel_name: convert_float8_to_int8_rtp
arg_in:  1 float8 -23.67 -23.50 -23.35 -23.0 23.0 23.35 23.50 23.67
arg_out: 0 buffer int8[1] -23 -23 -23 -23 23 24 24 24

[test]
name: Convert float8 to int8, round to negative infinity (floor)
kernel_name: convert_float8_to_int8_rtn
arg_in:  1 float8 -23.67 -23.50 -23.35 -23.0 23.0 23.35 23.50 23.67
arg_out: 0 buffer int8[1] -24 -24 -24 -23 23 23 23 23

[test]
name: Convert float8 to int8, round (half) to nearest even
kernel_name: convert_float8_to_int8_rte
arg_in:  1 float8 -23.67 -23.50 -23.35 -23.0 23.0 23.35 23.50 23.67
arg_out: 0 buffer int8[1]  -24 -24 -23 -23 23 23 24 24

[test]
name: Convert float8 to int8, round (default rounding mode, rtz)
kernel_name: convert_float8_to_int8
arg_in:  1 float8 -23.67 -23.50 -23.35 -23.0 23.0 23.35 23.50 23.67
arg_out: 0 buffer int8[1] -23 -23 -23 -23 23 23 23 23

[test]
name: Convert float4 to int4, round to zero
kernel_name: convert_float4_to_int4_rtz
arg_in:  1 float4 -123.1 0.9 1.3 123.456
arg_out: 0 buffer int4[1]  -123 0 1 123

[test]
name: Convert float4 to int4, round to positive infinity
kernel_name: convert_float4_to_int4_rtp
arg_in:  1 float4 -123.1 0.9 1.3 123.456
arg_out: 0 buffer int4[1]  -123 1 2 124

[test]
name: Convert float4 to int4, round to negative infinity
kernel_name: convert_float4_to_int4_rtn
arg_in:  1 float4 -123.1 0.9 1.3 123.456
arg_out: 0 buffer int4[1]  -124 0 1 123

[test]
name: Convert int4 to uint4 via cast, no rounding, no saturation
kernel_name: int4_as_uint4
arg_in:  1 int4 -123456 1 2 123456
arg_out: 0 buffer uint4[1]  4294843840 1 2 123456

!*/

kernel void convert_float8_to_int8_rtz(global int8* out, float8 in){
    *out = convert_int8_rtz(in);
}

kernel void convert_float8_to_int8_rtp(global int8* out, float8 in){
    *out = convert_int8_rtp(in);
}

kernel void convert_float8_to_int8_rtn(global int8* out, float8 in){
    *out = convert_int8_rtn(in);
}

kernel void convert_float8_to_int8_rte(global int8* out, float8 in){
    *out = convert_int8_rte(in);
}

/* 
    According to the following, default for float -> int is truncate:
    http://www.khronos.org/registry/cl/sdk/1.0/docs/man/xhtml/SELECT_ROUNDING_MODE.html 
*/
kernel void convert_float8_to_int8(global int8* out, float8 in){
    *out = convert_int8(in);
}

kernel void int4_as_uint4(global uint4* out, int4 in){
    *out = as_uint4(in);
}

kernel void convert_float4_to_int4_rtz(global int4* out, float4 in){
    *out = convert_int4_rtz(in);
}

kernel void convert_float4_to_int4_rtp(global int4* out, float4 in){
    *out = convert_int4_rtp(in);
}

kernel void convert_float4_to_int4_rtn(global int4* out, float4 in){
    *out = convert_int4_rtn(in);
}

kernel void convert_int4_to_char4_sat_rte(global char4* out, int4 in){
    *out = convert_char4_sat_rte(in);
}

kernel void convert_int4_to_char4_sat(global char4* out, int4 in){
    *out = convert_char4_sat(in);
}

kernel void convert_int_to_int4(global int4* out, int in){
    *out = (int4) in;
}

kernel void convert_char4_int4(global int4* out, char4 in){
    *out = convert_int4(in);
}

kernel void convert_int4_float4(global float4* out, int4 in){
    *out = convert_float4(in);
}
