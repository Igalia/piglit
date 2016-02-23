/*!
[config]
name: ilogb
clc_version_min: 10
dimensions: 1

[test]
name: float scalar ilogb(0.0)
kernel_name: ilogb_macro_ilogb0
global_size: 1 0 0
arg_out: 0 buffer int[1] 0
arg_in: 1 buffer float[1] 0.0

[test]
name: float scalar ilogb(nan)
kernel_name: ilogb_macro_ilogbnan
global_size: 1 0 0
arg_out: 0 buffer int[1] 0
arg_in: 1 buffer float[1] nan

!*/

kernel void ilogb_macro_ilogb0(global int* out, global float* in1) {
    size_t id = get_global_id(0);
    out[id] = ilogb(in1[id]) - FP_ILOGB0;
}

kernel void ilogb_macro_ilogbnan(global int* out, global float* in1) {
    size_t id = get_global_id(0);
    out[id] = ilogb(in1[id]) - FP_ILOGBNAN;
}
