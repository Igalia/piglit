/*!
[config]
name: fdiv with neg or abs inputs
clc_version_min: 10
require_device_extensions: cl_khr_fp64

dimensions: 1
global_size: 12 0 0

## Division ##


[test]
name: fdiv -x, y
kernel_name: fdiv_neg_x_y_f64

arg_out: 0 buffer double[12] \
 -1.0  -1.0   1.0   1.0     \
 -2.0   2.0   2.0  -2.0     \
 -0.5   0.5   0.5  -0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0   -1.0   \
  4.0   4.0  -4.0   -4.0   \
  2.0   2.0  -2.0   -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv x, -y
kernel_name: fdiv_x_neg_y_f64

arg_out: 0 buffer double[12] \
 -1.0  -1.0   1.0   1.0     \
 -2.0   2.0   2.0  -2.0     \
 -0.5   0.5   0.5  -0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0   -1.0   \
  4.0   4.0  -4.0   -4.0   \
  2.0   2.0  -2.0   -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv -x, -y
kernel_name: fdiv_neg_x_neg_y_f64

arg_out: 0 buffer double[12] \
  1.0   1.0  -1.0  -1.0     \
  2.0  -2.0  -2.0   2.0     \
  0.5  -0.5  -0.5   0.5

arg_in: 1 buffer double[12]  \
  1.0  -1.0   1.0   -1.0    \
  4.0   4.0  -4.0   -4.0    \
  2.0   2.0  -2.0   -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv |x|, y
kernel_name: fdiv_abs_x_y_f64

arg_out: 0 buffer double[12] \
  1.0  -1.0  -1.0   1.0     \
  2.0  -2.0   2.0  -2.0     \
  0.5  -0.5   0.5  -0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0   -1.0   \
  4.0   4.0  -4.0   -4.0   \
  2.0   2.0  -2.0   -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv x, |y|
kernel_name: fdiv_x_abs_y_f64

arg_out: 0 buffer double[12] \
  1.0  -1.0   1.0  -1.0     \
  2.0   2.0  -2.0  -2.0     \
  0.5   0.5  -0.5  -0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0   -1.0   \
  4.0   4.0  -4.0   -4.0   \
  2.0   2.0  -2.0   -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv |x|, |y|
kernel_name: fdiv_abs_x_abs_y_f64

arg_out: 0 buffer double[12] \
  1.0   1.0  1.0   1.0      \
  2.0   2.0  2.0   2.0      \
  0.5   0.5  0.5   0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0  -1.0    \
  4.0   4.0  -4.0  -4.0    \
  2.0   2.0  -2.0  -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv -|x|, y
kernel_name: fdiv_neg_abs_x_y_f64

arg_out: 0 buffer double[12] \
 -1.0   1.0   1.0  -1.0     \
 -2.0   2.0  -2.0   2.0     \
 -0.5   0.5  -0.5   0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0  -1.0    \
  4.0   4.0  -4.0  -4.0    \
  2.0   2.0  -2.0  -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv x, -|y|
kernel_name: fdiv_x_neg_abs_y_f64

arg_out: 0 buffer double[12] \
 -1.0   1.0  -1.0   1.0     \
 -2.0  -2.0   2.0   2.0     \
 -0.5  -0.5   0.5   0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0  -1.0    \
  4.0   4.0  -4.0  -4.0    \
  2.0   2.0  -2.0  -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0

[test]
name: fdiv -|x|, -|y|
kernel_name: fdiv_neg_abs_x_neg_abs_y_f64

arg_out: 0 buffer double[12] \
  1.0   1.0  1.0   1.0      \
  2.0   2.0  2.0   2.0      \
  0.5   0.5  0.5   0.5

arg_in: 1 buffer double[12] \
  1.0  -1.0   1.0  -1.0    \
  4.0   4.0  -4.0  -4.0    \
  2.0   2.0  -2.0  -2.0

arg_in: 2 buffer double[12] \
  1.0  -1.0  -1.0   1.0    \
  2.0  -2.0   2.0  -2.0    \
  4.0  -4.0   4.0  -4.0


[test]
name: fdiv 4.0, y
kernel_name: fdiv_4_y_f64

arg_out: 0 buffer double[12] \
  4.0  -4.0  8.0  -8.0       \
  1.0   1.0 -1.0  -1.0       \
  2.0  -2.0  nan  0.0

arg_in: 1 buffer double[12] \
  1.0  -1.0   0.5  -0.5    \
  4.0   4.0  -4.0  -4.0    \
  2.0  -2.0   0.0  inf

!*/

#pragma OPENCL EXTENSION cl_khr_fp64 : enable


kernel void fdiv_neg_x_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = -in0[id] / in1[id];
}

kernel void fdiv_x_neg_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = in0[id] / -in1[id];
}

kernel void fdiv_neg_x_neg_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = -in0[id] / -in1[id];
}

kernel void fdiv_abs_x_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = fabs(in0[id]) / in1[id];
}

kernel void fdiv_x_abs_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = in0[id] / fabs(in1[id]);
}

kernel void fdiv_abs_x_abs_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = fabs(in0[id]) / fabs(in1[id]);
}

kernel void fdiv_neg_abs_x_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = -fabs(in0[id]) / in1[id];
}

kernel void fdiv_x_neg_abs_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = in0[id] / -fabs(in1[id]);
}

kernel void fdiv_neg_abs_x_neg_abs_y_f64(global double* out, global double* in0, global double* in1)
{
    int id = get_global_id(0);
    out[id] = -fabs(in0[id]) / -fabs(in1[id]);
}

kernel void fdiv_4_y_f64(global double* out, global double* in0)
{
    int id = get_global_id(0);
    out[id] = 4.0 / in0[id];
}
