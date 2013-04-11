/*!
[config]

[test]
name: 1D 64 (8)
dimensions: 1
global_size: 64 0 0
local_size: 8 0 0
kernel_name: group_id_1d
arg_out: 0 buffer int[64] \
            0 0 0 0 0 0 0 0 \
            1 1 1 1 1 1 1 1 \
            2 2 2 2 2 2 2 2 \
            3 3 3 3 3 3 3 3 \
            4 4 4 4 4 4 4 4 \
            5 5 5 5 5 5 5 5 \
            6 6 6 6 6 6 6 6 \
            7 7 7 7 7 7 7 7

[test]
name: 2D (4 x 4), (2 x 2)
dimensions: 2
global_size: 4 4 0
local_size:  2 2 0
kernel_name: group_id_2d
arg_out: 0 buffer int[16] \
            0x00 0x00 0x10 0x10 \
            0x00 0x00 0x10 0x10 \
            0x01 0x01 0x11 0x11 \
            0x01 0x01 0x11 0x11

[test]
name: 3D (4 x 4 x 4), (2 x 2 x 2)
dimensions: 3
global_size: 4 4 4
local_size:  2 2 2
kernel_name: group_id_3d
arg_out: 0 buffer int[64] \
  0x000 0x000 0x100 0x100 \
  0x000 0x000 0x100 0x100 \
  0x010 0x010 0x110 0x110 \
  0x010 0x010 0x110 0x110 \
    0x000 0x000 0x100 0x100 \
    0x000 0x000 0x100 0x100 \
    0x010 0x010 0x110 0x110 \
    0x010 0x010 0x110 0x110 \
      0x001 0x001 0x101 0x101 \
      0x001 0x001 0x101 0x101 \
      0x011 0x011 0x111 0x111 \
      0x011 0x011 0x111 0x111 \
        0x001 0x001 0x101 0x101 \
        0x001 0x001 0x101 0x101 \
        0x011 0x011 0x111 0x111 \
        0x011 0x011 0x111 0x111
!*/

kernel void group_id_1d(global int *out) {
	out[get_global_id(0)] = get_group_id(0);
}

kernel void group_id_2d(global int *out) {
	out[get_global_id(0) + get_global_id(1) * get_global_size(0)] =
				(get_group_id(0) << 4) | get_group_id(1);
}

kernel void group_id_3d(global int *out) {
	out[get_global_id(0) + (get_global_id(1) * get_global_size(0)) +
           (get_global_id(2) * get_global_size(0) * get_global_size(1))] =

	(get_group_id(0) << 8) | (get_group_id(1) << 4) | get_group_id(2);
}
