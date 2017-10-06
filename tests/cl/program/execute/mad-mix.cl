/*!

[config]
name: f32 mad with conversion from f16
clc_version_min: 10
build_options: -cl-denorms-are-zero
require_device_extensions: cl_khr_fp16

dimensions: 1

[test]
name: mad mix f32 f16lo f16lo f16lo
kernel_name: mad_mix_f32_f16lo_f16lo_f16lo
global_size: 4 0 0

arg_out: 0 buffer float[4] \
  0.0   1.0   1.0  -1.0

arg_in: 1 buffer half[4] \
  0.0   1.0   0.0  -1.0

arg_in: 2 buffer half[4] \
  0.0   1.0   1.0   1.0

arg_in: 3 buffer half[4] \
  0.0   0.0   1.0   0.0


[test]
name: mad mix f32 fneg(f16lo) f16lo f16lo
kernel_name: mad_mix_f32_negf16lo_f16lo_f16lo
global_size: 4 0 0

arg_out: 0 buffer float[4] \
  0.0  -1.0   1.0   1.0

arg_in: 1 buffer half[4] \
  0.0   1.0   0.0  -1.0

arg_in: 2 buffer half[4] \
  0.0   1.0   1.0   1.0

arg_in: 3 buffer half[4] \
  0.0   0.0   1.0   0.0


[test]
name: mad mix f32 f16lo f16lo f16hi
kernel_name: mad_mix_f32_f16lo_f16lo_f16hi
global_size: 4 0 0

arg_out: 0 buffer float[4] \
  0.0                      \
  1.0                      \
  1.0                      \
 -1.0

arg_in: 1 buffer half[4] \
  0.0   \
  1.0   \
  0.0   \
 -1.0

arg_in: 2 buffer half[4] \
  0.0                    \
  1.0                    \
  1.0                    \
  1.0

arg_in: 3 buffer half2[4] \
  1000.0     0.0          \
  1000.0     0.0          \
  1000.0     1.0          \
  1000.0     0.0


[test]
name: mad mix f32 f16lo f16lo neg(f16hi)
kernel_name: mad_mix_f32_f16lo_f16lo_negf16hi
global_size: 5 0 0

arg_out: 0 buffer float[5] \
  0.0                      \
  1.0                      \
 -1.0                      \
 -1.0                      \
  0.0

arg_in: 1 buffer half[5] \
  0.0                    \
  1.0                    \
  0.0                    \
 -1.0                    \
  2.0

arg_in: 2 buffer half[5] \
  0.0                    \
  1.0                    \
  1.0                    \
  1.0                    \
  2.0

arg_in: 3 buffer half2[5] \
  1000.0     0.0          \
  1000.0     0.0          \
  1000.0     1.0          \
  1000.0     0.0          \
  1000.0     4.0


[test]
name: mad mix f16lo fneg(f16lo) f16lo f16lo
kernel_name: mad_mix_f16lo_negf16lo_f16lo_f16lo
global_size: 4 0 0

arg_out: 0 buffer half[4] \
  0.0  -1.0   1.0   1.0

arg_in: 1 buffer half[4] \
  0.0   1.0   0.0  -1.0

arg_in: 2 buffer half[4] \
  0.0   1.0   1.0   1.0

arg_in: 3 buffer half[4] \
  0.0   0.0   1.0   0.0


[test]
name: mad mix f16hi fneg(f16lo) f16lo f16lo
kernel_name: mad_mix_f16hi_negf16lo_f16lo_f16lo
global_size: 4 0 0

arg_out: 0 buffer half2[4] \
  2.0    0.0               \
  2.0   -1.0               \
  2.0    1.0               \
  2.0    1.0

arg_in: 1 buffer half[4] \
  0.0   1.0   0.0  -1.0

arg_in: 2 buffer half[4] \
  0.0   1.0   1.0   1.0

arg_in: 3 buffer half[4] \
  0.0   0.0   1.0   0.0



[test]
name: mad mix f32 f16lo f16lo f16lo with clamp
kernel_name: mad_mix_f32_f16lo_f16lo_f16lo_clamp
global_size: 5 0 0

arg_out: 0 buffer float[5] \
  0.0   1.0   0.0   0.75   \
  1.0

arg_in: 1 buffer half[5] \
  0.0   2.0  -2.0   0.5  \
  0.5

arg_in: 2 buffer half[5] \
  0.0   1.0   1.0   0.5  \
  1.0

arg_in: 3 buffer half[5] \
  0.0   1.0   1.0   0.5  \
  0.5


[test]
name: mad mix f16lo f16lo f16lo f16lo with clamp
kernel_name: mad_mix_f16lo_f16lo_f16lo_f16lo_clamp
global_size: 5 0 0

arg_out: 0 buffer half[5] \
  0.0   1.0   0.0   0.75  \
  1.0

arg_in: 1 buffer half[5] \
  0.0   2.0  -2.0   0.5  \
  0.5

arg_in: 2 buffer half[5] \
  0.0   1.0   1.0   0.5  \
  1.0

arg_in: 3 buffer half[5] \
  0.0   1.0   1.0   0.5  \
  0.5


[test]
name: mad mix f16hi f16lo f16lo f16lo with clamp
kernel_name: mad_mix_f16hi_f16lo_f16lo_f16lo_clamp
global_size: 5 0 0

arg_out: 0 buffer half2[5] \
  2.0  0.0                 \
  2.0  1.0                 \
  2.0  0.0                 \
  2.0  0.75                \
  2.0  1.0

arg_in: 1 buffer half[5] \
  0.0   2.0  -2.0   0.5  \
  0.5

arg_in: 2 buffer half[5] \
  0.0   1.0   1.0   0.5  \
  1.0

arg_in: 3 buffer half[5] \
  0.0   1.0   1.0   0.5  \
  0.5


!*/

#pragma OPENCL EXTENSION cl_khr_fp16 : enable

kernel void mad_mix_f32_f16lo_f16lo_f16lo(global float* out, global half* in0, global half* in1, global half* in2)
{
    int id = get_global_id(0);
    out[id] = (float)in0[id] * (float)in1[id] + (float)in2[id];
}

kernel void mad_mix_f32_negf16lo_f16lo_f16lo(global float* out, global half* in0, global half* in1, global half* in2)
{
    int id = get_global_id(0);
    out[id] = (float)-in0[id] * (float)in1[id] + (float)in2[id];
}

kernel void mad_mix_f32_f16lo_f16lo_f16hi(global float* out, global half* in0, global half* in1, volatile global half2* in2)
{
    int id = get_global_id(0);
    out[id] = (float)in0[id] * (float)in1[id] + (float)in2[id].y;
}

kernel void mad_mix_f32_f16lo_f16lo_negf16hi(global float* out, global half* in0, global half* in1, volatile global half2* in2)
{
    int id = get_global_id(0);
    out[id] = (float)in0[id] * (float)in1[id] + (float)-in2[id].y;;
}

kernel void mad_mix_f16lo_negf16lo_f16lo_f16lo(global half* out, global half* in0, global half* in1, global half* in2)
{
    int id = get_global_id(0);
    float mad = (float)-in0[id] * (float)in1[id] + (float)in2[id];
    out[id] = (half)mad;
}

kernel void mad_mix_f16hi_negf16lo_f16lo_f16lo(volatile global half2* out, global half* in0, global half* in1, global half* in2)
{
    int id = get_global_id(0);
    float mad = (float)-in0[id] * (float)in1[id] + (float)in2[id];
    half2 result = { 2.0h, (half)mad };
    out[id] = result;
}

kernel void mad_mix_f32_f16lo_f16lo_f16lo_clamp(global float* out, global half* in0, global half* in1, global half* in2)
{
    int id = get_global_id(0);
    float mad = (float)in0[id] * (float)in1[id] + (float)in2[id];
    out[id] = clamp(mad, 0.0f, 1.0f);
}

kernel void mad_mix_f16lo_f16lo_f16lo_f16lo_clamp(global half* out, global half* in0, global half* in1, global half* in2)
{
    int id = get_global_id(0);
    float mad = (float)in0[id] * (float)in1[id] + (float)in2[id];
    out[id] = clamp((half)mad, 0.0h, 1.0h);
}

kernel void mad_mix_f16hi_f16lo_f16lo_f16lo_clamp(volatile global half2* out, global half* in0, global half* in1, global half* in2)
{
    int id = get_global_id(0);
    float mad = (float)in0[id] * (float)in1[id] + (float)in2[id];
    half2 result = { 2.0h, clamp((half)mad, 0.0h, 1.0h) };
    out[id] = result;
}
