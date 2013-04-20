/*!
[config]
name: get_global_id
clc_version_min: 10

[test]
name: 1D, global_size 4 0 0
kernel_name: fill1d
dimensions: 1
global_size: 4 0 0
local_size: 1 0 0
arg_out: 0 buffer int[4] 4 4 4 4

[test]
name: 1D, global_size 4 0 0, local_size 2 0 0
kernel_name: fill1d
dimensions: 1
global_size: 4 0 0
local_size: 2 0 0
arg_out: 0 buffer int[4] 4 4 4 4

[test]
name: 1D, global_size 4 0 0, local_size 4 0 0
kernel_name: fill1d
dimensions: 1
global_size: 4 0 0
local_size: 4 0 0
arg_out: 0 buffer int[4] 4 4 4 4

[test]
name: 2D, global_size 4 4 0, local_size 1 1 0
kernel_name: fill2d
dimensions: 2
global_size: 4 4 0
local_size: 1 1 0
arg_out: 0 buffer int[16] 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16

[test]
name: 2D, global_size 4 4 0, local_size 2 2 0
kernel_name: fill2d
dimensions: 2
global_size: 4 4 0
local_size: 2 2 0
arg_out: 0 buffer int[16] 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16

[test]
name: 2D, global_size 4 4 0, local_size 4 4 0
kernel_name: fill2d
dimensions: 2
global_size: 4 4 0
local_size: 4 4 0
arg_out: 0 buffer int[16] 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16 16

[test]
name: 3D, global_size 4 4 4, local_size 1 1 1
kernel_name: fill3d
dimensions: 3
global_size: 4 4 4
local_size: 1 1 1
arg_out: 0 buffer int[64] 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64

[test]
name: 3D, global_size 4 4 4, local_size 2 2 2
kernel_name: fill3d
dimensions: 3
global_size: 4 4 4
local_size: 2 2 2
arg_out: 0 buffer int[64] 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64

[test]
name: 3D, global_size 4 4 4, local_size 4 4 4
kernel_name: fill3d
dimensions: 3
global_size: 4 4 4
local_size: 4 4 4
arg_out: 0 buffer int[64] 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 \
                          64 64 64 64 64 64 64 64 64 64 64 64 64 64 64 64
!*/

kernel void fill1d(global int* out) {
    unsigned int size = get_global_size(0);
	unsigned int id = get_global_id(0);
    out[id] = size;
}

kernel void fill2d(global int* out) {
    unsigned int size = get_global_size(0) * get_global_size(1);
    unsigned int id = get_global_id(0)*get_global_size(1)+get_global_id(1);
	out[id] = size;
}

kernel void fill3d(global int* out) {
    unsigned int size = get_global_size(0) * get_global_size(1) * get_global_size(2);
	unsigned int id = (get_global_id(0) * (get_global_size(1) * get_global_size(2)))
        + (get_global_id(1) * get_global_size(2)) + get_global_id(2);
    out[id] = size;
}
