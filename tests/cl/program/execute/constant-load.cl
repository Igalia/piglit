/*!
[config]

[test]
kernel_name: constant_load_global_store_2
name: constant load global write int2
arg_out: 0 buffer int2[1] 1 2
arg_in:  1 buffer int2[1] 1 2

[test]
kernel_name: constant_load_global_store_4
name: constant load global write int4
arg_out: 0 buffer int4[1] 1 2 3 4
arg_in:  1 buffer int4[1] 1 2 3 4

!*/

kernel void constant_load_global_store_2(global int2 *out, constant int2 *in){
    out[0] = *in;
}

kernel void constant_load_global_store_4(global int4 *out, constant int4 *in){
    out[0] = *in;
}
