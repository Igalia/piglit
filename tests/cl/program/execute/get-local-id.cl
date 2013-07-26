/*!
[config]
name: get_local_id
clc_version_min: 10

[test]
name: 1D, global_size 4 0 0, local_size 1 0 0
kernel_name: fill1d
dimensions: 1
global_size: 4 0 0
local_size: 1 0 0
arg_out: 0 buffer int[4] 0 0 0 0

[test]
name: 1D, global_size 4 0 0, local_size 2 0 0
kernel_name: fill1d
dimensions: 1
global_size: 4 0 0
local_size: 2 0 0
arg_out: 0 buffer int[4] 0 1 0 1

[test]
name: 1D, global_size 4 0 0, local_size 4 0 0
kernel_name: fill1d
dimensions: 1
global_size: 4 0 0
local_size: 4 0 0
arg_out: 0 buffer int[4] 0 1 2 3

[test]
name: 2D, global_size 4 4 0, local_size 1 1 0
kernel_name: fill2d
dimensions: 2
global_size: 4 4 0
local_size: 1 1 0
arg_out: 0 buffer int[16] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

[test]
name: 2D, global_size 4 4 0, local_size 2 2 0
kernel_name: fill2d
dimensions: 2
global_size: 4 4 0
local_size: 2 2 0
arg_out: 0 buffer int[16] 0 10 0 10 1 11 1 11 0 10 0 10 1 11 1 11

[test]
name: 2D, global_size 4 4 0, local_size 4 4 0
kernel_name: fill2d
dimensions: 2
global_size: 4 4 0
local_size: 4 4 0
arg_out: 0 buffer int[16] 0 10 20 30 1 11 21 31 2 12 22 32 3 13 23 33

[test]
name: 3D, global_size 4 4 4, local_size 1 1 1
kernel_name: fill3d
dimensions: 3
global_size: 4 4 4
local_size: 1 1 1
arg_out: 0 buffer int[64] 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
                          0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
                          0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 \
                          0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0

[test]
name: 3D, global_size 4 4 4, local_size 2 2 2
kernel_name: fill3d
dimensions: 3
global_size: 4 4 4
local_size: 2 2 2
arg_out: 0 buffer int[64]  0 100 0 100 10 110 10 110 0 100 0 100 10 110 10 110 \
                           1 101 1 101 11 111 11 111 1 101 1 101 11 111 11 111 \
                           0 100 0 100 10 110 10 110 0 100 0 100 10 110 10 110 \
                           1 101 1 101 11 111 11 111 1 101 1 101 11 111 11 111

[test]
name: 3D, global_size 4 4 4, local_size 4 4 4
kernel_name: fill3d
dimensions: 3
global_size: 4 4 4
local_size: 4 4 4
arg_out: 0 buffer int[64] 0 100 200 300 10 110 210 310 20 120 220 320 30 130 230 330 \
                          1 101 201 301 11 111 211 311 21 121 221 321 31 131 231 331 \
                          2 102 202 302 12 112 212 312 22 122 222 322 32 132 232 332 \
                          3 103 203 303 13 113 213 313 23 123 223 323 33 133 233 333

[test]
name: 3D, global_size 4 4 4, local_size 4 2 1
kernel_name: fill3d
dimensions: 3
global_size: 4 4 4
local_size: 4 2 1
arg_out: 0 buffer int[64]  0 100 200 300 10 110 210 310 0 100 200 300 10 110 210 310 \
                           0 100 200 300 10 110 210 310 0 100 200 300 10 110 210 310 \
                           0 100 200 300 10 110 210 310 0 100 200 300 10 110 210 310 \
                           0 100 200 300 10 110 210 310 0 100 200 300 10 110 210 310

!*/

kernel void fill1d(global int* out) {
	unsigned int id = get_global_id(0);
	out[id] = get_local_id(0);
}

kernel void fill2d(global int* out) {
	unsigned int id = get_global_id(0) + 4*get_global_id(1);
	out[id] = 10*get_local_id(0) + get_local_id(1);
}

kernel void fill3d(global int* out) {
	unsigned int id = get_global_id(0) + 4*get_global_id(1) + 16*get_global_id(2);
	out[id] = 100*get_local_id(0) + 10*get_local_id(1) + get_local_id(2);
}
