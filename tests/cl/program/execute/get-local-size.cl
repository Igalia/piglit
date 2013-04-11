/*!
[config]

[test]
name: 1D 4 (2)
dimensions: 1
global_size: 4 0 0
local_size: 2 0 0
kernel_name: local_size_1d
arg_out: 0 buffer int[4] repeat 2

[test]
name: 2D 4 x 4 (2 x 1)
dimensions: 2
global_size: 4 4 0
local_size: 2 1 0
kernel_name: local_size_2d
arg_out: 0 buffer int[16] repeat 0x21

[test]
name: 3D 8 x 4 x 2 (4 x 2 x 1)
dimensions: 3
global_size: 8 4 2
local_size:  4 2 1
kernel_name: local_size_3d
arg_out: 0 buffer int[64] repeat 0x421

!*/

kernel void local_size_1d(global int *out) {
	out[get_global_id(0)] = get_local_size(0);
}

kernel void local_size_2d(global int *out) {
	out[get_global_id(0) + get_global_id(1) * get_global_size(0)] =
				(get_local_size(0) << 4) | get_local_size(1);
}

kernel void local_size_3d(global int *out) {
	out[get_global_id(0) + (get_global_id(1) * get_global_size(0)) +
           (get_global_id(2) * get_global_size(0) * get_global_size(1))] =

	(get_local_size(0) << 8) | (get_local_size(1) << 4) | get_local_size(2);
}
