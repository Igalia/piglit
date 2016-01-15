/*!
[config]

[test]
kernel_name: constant_load_global_store_1
name: constant load global write int
arg_out: 0 buffer int[1] 1
arg_in:  1 buffer int[1] 1

[test]
kernel_name: constant_load_global_store_2
name: constant load global write int2
arg_out: 0 buffer int2[1] 2 3
arg_in:  1 buffer int2[1] 2 3

[test]
kernel_name: constant_load_global_store_4
name: constant load global write int4
arg_out: 0 buffer int4[1] 4 5 6 7
arg_in:  1 buffer int4[1] 4 5 6 7

!*/

kernel void constant_load_global_store_1(global int2 *out, constant int *in){
    out[0] = *in;
}

kernel void constant_load_global_store_2(global int2 *out, constant int2 *in){
    out[0] = *in;
}

kernel void constant_load_global_store_4(global int4 *out, constant int4 *in){
    out[0] = *in;
}
