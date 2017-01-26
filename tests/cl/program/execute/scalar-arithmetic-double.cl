/*!
[config]
name: fneg f64
clc_version_min: 10
require_device_extensions: cl_khr_fp64
dimensions: 1

## Unary minus ##

[test]
name: minus
kernel_name: minus
global_size: 20 0 0

arg_out: 0 buffer double[20]       \
   -0.0      0.0    -0.5    0.5    \
   -1.0      1.0    -2.0    2.0    \
   -4.0      4.0   -10.0    10.0   \
   -inf      inf     nan   -345.25 \
 345.25 -455.125  455.125  0.12345

arg_in: 1 buffer double[20]          \
  0.0     -0.0           0.5     -0.5 \
  1.0     -1.0           2.0     -2.0 \
  4.0     -4.0          10.0    -10.0 \
  inf     -inf           nan   345.25 \
 -345.25   455.125  -455.125 -0.12345

!*/

#pragma OPENCL EXTENSION cl_khr_fp64 : enable

kernel void minus(global double* out, global double* in) {
    int id = get_global_id(0);
    out[id] = -in[id];
}
