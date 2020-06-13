/*!
[config]
name: short inline immediates
clc_version_min: 10
build_options: -cl-denorms-are-zero
require_device_extensions: cl_khr_fp16

dimensions: 1
global_size: 10 0 0
local_size: 10 0 0

## Addition ##

[test]
name: add 0.5
kernel_name: add_fp_half
arg_out: 0 buffer short[10] \
  0x3800  0x3801   0x3802    0x37ff   0xb800 \
  0xb7ff  0x7800   0xf800    0x7c00   0x8fb0

arg_in: 1 buffer short[10]		     \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add 1.0
kernel_name: add_fp_1
arg_out: 0 buffer short[10] \
  0x3c00  0x3c01   0x3c02    0x3bff   0xbc00 \
  0xbbff  0x7c00   0xfc00    0x8000   0x93b0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add 2.0
kernel_name: add_fp_2
arg_out: 0 buffer short[10] \
  0x8400  0x8401   0x8402    0x3fff   0xc000 \
  0xbfff  0x8000   0x0000    0x8400   0x97b0

arg_in: 1 buffer short[10]  \
  0x4400  0x4401   0x4402    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0


[test]
name: add 4.0
kernel_name: add_fp_4
arg_out: 0 buffer short[10] \
  0x4400  0x4401   0x4402    0x43ff   0xc400 \
  0xc3ff  0x8400   0x0400    0x8800   0x9bb0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0


[test]
name: add rcp(2pi)
kernel_name: add_inv_2pi
arg_out: 0 buffer short[10] \
  0x3118  0x3119  0x311a  0x3117  0xb118 \
  0xb117  0x7118  0xf118  0x7518  0x88c8

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add -0.5
kernel_name: add_neg_fp_half
arg_out: 0 buffer short[10] \
  0xb800  0xb801   0xb802   0xb7ff   0x3800 \
  0x37ff  0xf800   0x7800   0xfc00   0x0fb0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add -1.0
kernel_name: add_neg_fp_1
arg_out: 0 buffer short[10] \
  0xbc00  0xbc01   0xbc02    0xbbff   0x3c00 \
  0x3bff  0xfc00   0x7c00    0x0000   0x13b0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add -2.0
kernel_name: add_neg_fp_2
arg_out: 0 buffer short[10] \
  0xc000  0xc001   0xc002    0xbfff   0x4000 \
  0x3fff  0x0000   0x8000    0x0400   0x17b0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add -4.0
kernel_name: add_neg_fp_4
arg_out: 0 buffer short[10] \
  0xc400  0xc401   0xc402    0xc3ff   0x4400 \
  0x43ff  0x0400   0x8400    0x0800   0x1bb0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add integer -16
kernel_name: add_i16_neg16
arg_out: 0 buffer short[10] \
   0xfff0  0xfff1  0xfff2  0xffef  0x7ff0 \
   0x7fef  0x3ff0  0xbff0  0x43f0  0x57a0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add integer 64
kernel_name: add_i16_64
arg_out: 0 buffer short[10] \
   0x0040  0x0041  0x0042  0x003f  0x8040 \
   0x803f  0x4040  0xc040  0x4440  0x57f0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add integer 1
kernel_name: add_i16_1
arg_out: 0 buffer short[10] \
   0x0001  0x0002  0x0003  0x0000  0x8001 \
   0x8000  0x4001  0xc001  0x4401  0x57b1

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add fp other
kernel_name: add_fp_other
arg_out: 0 buffer short[10] \
  0x4900  0x4901   0x4902    0x48ff   0xc900 \
  0xc8ff  0x8900   0x0900    0x8d00   0xa0b0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

[test]
name: add neg fp other
kernel_name: add_neg_fp_other

arg_out: 0 buffer short[10]   \
  0xc900  0xc901   0xc902    0xc8ff   0x4900 \
  0x48ff  0x0900   0x8900    0x0d00   0x20b0

arg_in: 1 buffer short[10]  \
  0x0000  0x0001   0x0002    0xffff   0x8000 \
  0x7fff  0x4000   0xc000    0x4400   0x57b0

!*/

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

kernel void add_fp_half(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(0.5h);
}

kernel void add_fp_1(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(1.0h);
}

kernel void add_fp_2(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(2.0h);
}

kernel void add_fp_4(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(4.0h);
}

kernel void add_inv_2pi(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + (short)0x3118;
}

kernel void add_neg_fp_half(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(-0.5h);
}

kernel void add_neg_fp_1(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(-1.0h);
}

kernel void add_neg_fp_2(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(-2.0h);
}

kernel void add_neg_fp_4(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(-4.0h);
}

kernel void add_i16_neg16(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + (short)-16;
}

kernel void add_i16_64(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + (short)64;
}

kernel void add_i16_1(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + (short)1;
}

kernel void add_fp_other(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(10.0h);
}

kernel void add_neg_fp_other(global short* out, global short* in)
{
    int id = get_global_id(0);
    out[id] = in[id] + as_short(-10.0h);
}
