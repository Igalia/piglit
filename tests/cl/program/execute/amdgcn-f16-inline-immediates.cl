/*!
[config]
name: f16 inline immediate values
clc_version_min: 10
build_options: -cl-denorms-are-zero
require_device_extensions: cl_khr_fp16

dimensions: 1
global_size: 10 0 0

## Addition ##

[test]
name: add 0.5
kernel_name: add_half
arg_out: 0 buffer half[10] \
  0.5   1.0    0.0   1.5   -0.5 \
  2.5   -1.5   4.5  -3.5  123.5

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add 1.0
kernel_name: add_1
arg_out: 0 buffer half[10] \
  1.0   1.5    0.5   2.0   0.0 \
  3.0   -1.0   5.0  -3.0  124.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add 2.0
kernel_name: add_2
arg_out: 0 buffer half[10] \
  2.0   2.5    1.5   3.0   1.0 \
  4.0   0.0    6.0  -2.0 125.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0


[test]
name: add 4.0
kernel_name: add_4
arg_out: 0 buffer half[10] \
  4.0   4.5    3.5   5.0   3.0 \
  6.0   2.0    8.0  0.0  127.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add rcp(2pi)
kernel_name: add_inv_2pi
arg_out: 0 buffer short[10] \
  0x3118  0x3946  0xB574  0x3CA3  0xBABA \
  0x4052  0xBF5D  0x4429  0xC3AE  0x57B3

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 0.5
kernel_name: sub_half
arg_out: 0 buffer half[10] \
 -0.5    0.0  -1.0   0.5   -1.5 \
  1.5   -2.5   3.5  -4.5  122.5

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 1.0
kernel_name: sub_1
arg_out: 0 buffer half[10] \
 -1.0   -0.5  -1.5   0.0   -2.0 \
  1.0   -3.0   3.0  -5.0  122.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 2.0
kernel_name: sub_2
arg_out: 0 buffer half[10] \
 -2.0   -1.5  -2.5  -1.0   -3.0 \
  0.0   -4.0   2.0  -6.0  121.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub 4.0
kernel_name: sub_4
arg_out: 0 buffer half[10] \
 -4.0   -3.5  -4.5  -3.0   -5.0 \
 -2.0   -6.0   0.0  -8.0  119.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add integer 64
kernel_name: add_i16_64
arg_out: 0 buffer short[10] \
   0x0040  0x3800  0xB800  0x3C00  0xBC00 \
   0x4000  0xC000  0x4400  0xC400  0x57B0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: add other
kernel_name: add_other
arg_out: 0 buffer half[10] \
 10.0  10.5    9.5  11.0   9.0 \
 12.0   8.0   14.0   6.0 133.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0   -2.0   4.0  -4.0  123.0

[test]
name: sub other
kernel_name: sub_other

arg_out: 0 buffer half[10]   \
-10.0  -9.5  -10.5  -9.0 -11.0 \
 -8.0 -12.0   -6.0  -14.0 113.0

arg_in: 1 buffer half[10]  \
  0.0   0.5   -0.5   1.0   -1.0 \
  2.0  -2.0    4.0  -4.0  123.0

!*/

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

kernel void add_half(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 0.5h;
}

kernel void add_1(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 1.0h;
}

kernel void add_2(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 2.0h;
}

kernel void add_4(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 4.0h;
}

kernel void add_inv_2pi(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_half((short)0x3118);
}

kernel void sub_half(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 0.5h;
}

kernel void sub_1(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 1.0h;
}

kernel void sub_2(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 2.0h;
}

kernel void sub_4(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 4.0h;
}

kernel void add_i16_neg16(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_half((short)-16);
}

kernel void add_i16_64(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_half((short)64);
}

kernel void add_i16_1(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_half((short)1);
}

kernel void add_other(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + 10.0h;
}

kernel void sub_other(global half* out, global half* in)
{
    int id = get_global_id(0);
    out[id] = in[id] - 10.0h;
}
