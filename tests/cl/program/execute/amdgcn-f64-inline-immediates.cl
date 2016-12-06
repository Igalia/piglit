/*!
[config]
name: SI double inline immediates
clc_version_min: 10
require_device_extensions: cl_khr_fp64

dimensions: 1
global_size: 10 0 0

## Addition ##

[test]
name: add 0.5
kernel_name: add_half
arg_out: 0 buffer double[10] \
  0.5   1.0    0.0   1.5   -0.5 \
  2.5   -1.5   4.5  -3.5  123.5

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add 1.0
kernel_name: add_1
arg_out: 0 buffer double[10] \
  1.0   1.5    0.5   2.0   0.0 \
  3.0   -1.0   5.0  -3.0  124.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add 2.0
kernel_name: add_2
arg_out: 0 buffer double[10] \
  2.0   2.5    1.5   3.0   1.0 \
  4.0   0.0    6.0  -2.0 125.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0


[test]
name: add 4.0
kernel_name: add_4
arg_out: 0 buffer double[10] \
  4.0   4.5    3.5   5.0   3.0 \
  6.0   2.0    8.0  0.0  127.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add rcp(2pi)
kernel_name: add_inv_2pi
arg_out: 0 buffer double[10] \
  0x1.45f306dc9c882p-3      0x1.517cc1b72722p-1   \
 -0x1.5d067c91b1bbfp-2      0x1.28be60db9391p+0   \
 -0x1.ae833e48d8dep-1       0x1.145f306dc9c88p+1  \
 -0x1.d7419f246c6fp+0       0x1.0a2f9836e4e44p+2  \
 -0x1.eba0cf9236378p+1      0x1.eca2f9836e4e4p+6

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 0.5
kernel_name: sub_half
arg_out: 0 buffer double[10] \
 -0.5    0.0  -1.0   0.5   -1.5 \
  1.5   -2.5   3.5  -4.5  122.5

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 1.0
kernel_name: sub_1
arg_out: 0 buffer double[10] \
 -1.0   -0.5  -1.5   0.0   -2.0 \
  1.0   -3.0   3.0  -5.0  122.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 2.0
kernel_name: sub_2
arg_out: 0 buffer double[10] \
 -2.0   -1.5  -2.5  -1.0   -3.0 \
  0.0   -4.0   2.0  -6.0  121.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 4.0
kernel_name: sub_4
arg_out: 0 buffer double[10] \
 -4.0   -3.5  -4.5  -3.0   -5.0 \
 -2.0   -6.0   0.0  -8.0  119.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add integer 64
kernel_name: add_i64_64
arg_out: 0 buffer double[10] \
  0x0.000000000004p-1022 0x1p-1 -0x1p-1 0x1p+0 -0x1p+0 \
  0x1p+1 -0x1p+1 0x1p+2 -0x1p+2 0x1.ecp+6

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add float 1.0
kernel_name: add_f32_1
arg_out: 0 buffer double[10] \
  0x0.000003f8p-1022 0x1p-1 -0x1p-1 0x1p+0 -0x1p+0 \
  0x1p+1 -0x1p+1 0x1p+2 -0x1p+2 0x1.ecp+6

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add float 4.0
kernel_name: add_f32_4
arg_out: 0 buffer double[10] \
  0x0.00000408p-1022 0x1p-1 -0x1p-1 0x1p+0 -0x1p+0 \
  0x1p+1 -0x1p+1 0x1p+2 -0x1p+2 0x1.ecp+6

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add other
kernel_name: add_other
arg_out: 0 buffer double[10] \
 10.0  10.5    9.5  11.0   9.0 \
 12.0   8.0   14.0   6.0 133.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub other
kernel_name: sub_other

arg_out: 0 buffer double[10]   \
-10.0  -9.5  -10.5  -9.0 -11.0 \
 -8.0 -12.0   -6.0  -14.0 113.0

arg_in: 1 buffer double[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0  -2.0    4.0  -4.0  123.0

!*/

#pragma OPENCL EXTENSION cl_khr_fp64 : enable


kernel void add_half(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 0.5;
}

kernel void add_1(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 1.0;
}

kernel void add_2(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 2.0;
}

kernel void add_4(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 4.0;
}

kernel void add_inv_2pi(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 0x1.45f306dc9c882p-3;
}

kernel void sub_half(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 0.5;
}

kernel void sub_1(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 1.0;
}

kernel void sub_2(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 2.0;
}

kernel void sub_4(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 4.0;
}

kernel void add_i64_neg16(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_double((long)-16);
}

kernel void add_i64_64(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_double((long)64);
}

kernel void add_i64_1(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_double((long)1);
}

kernel void add_f32_1(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_double((long)as_int(1.0f));
}

kernel void add_f32_4(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_double((long)as_int(4.0f));
}

kernel void add_other(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 10.0;
}

kernel void sub_other(global double* out, global double* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 10.0;
}
