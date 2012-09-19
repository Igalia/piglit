/*!
[config]
name: get_global_id
clc_version_min: 10

kernel_name: fill

[test]
name: 1D, global_size 4 0 0, local_size 1 0 0
dimensions: 1
global_size: 4 0 0
local_size: 1 0 0
arg_out: 0 buffer int[4] 0 1 2 3

[test]
name: 1D, global_size 4 0 0, local_size 2 0 0
dimensions: 1
global_size: 4 0 0
local_size: 2 0 0
arg_out: 0 buffer int[4] 0 1 2 3

[test]
name: 1D, global_size 4 0 0, local_size 4 0 0
dimensions: 1
global_size: 4 0 0
local_size: 4 0 0
arg_out: 0 buffer int[4] 0 1 2 3

[test]
name: 2D, global_size 4 4 0, local_size 1 1 0
dimensions: 2
global_size: 4 4 0
local_size: 1 1 0
arg_out: 0 buffer int[16] 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

[test]
name: 2D, global_size 4 4 0, local_size 2 2 0
dimensions: 2
global_size: 4 4 0
local_size: 2 2 0
arg_out: 0 buffer int[16] 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

[test]
name: 2D, global_size 4 4 0, local_size 4 4 0
dimensions: 2
global_size: 4 4 0
local_size: 4 4 0
arg_out: 0 buffer int[16] 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15

[test]
name: 3D, global_size 4 4 4, local_size 1 1 1
dimensions: 3
global_size: 4 4 4
local_size: 1 1 1
arg_out: 0 buffer int[64]  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 \
                          16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 \
                          32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 \
                          48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63

[test]
name: 3D, global_size 4 4 4, local_size 2 2 2
dimensions: 3
global_size: 4 4 4
local_size: 2 2 2
arg_out: 0 buffer int[64]  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 \
                          16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 \
                          32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 \
                          48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63

[test]
name: 3D, global_size 4 4 4, local_size 4 4 4
dimensions: 3
global_size: 4 4 4
local_size: 4 4 4
arg_out: 0 buffer int[64]  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14 15 \
                          16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 \
                          32 33 34 35 36 37 38 39 40 41 42 43 44 45 46 47 \
                          48 49 50 51 52 53 54 55 56 57 58 59 60 61 62 63
!*/

kernel void fill(global int* out) {
	unsigned int id = get_global_id(0) + 4*get_global_id(1) + 16*get_global_id(2);
	out[id] = id;
}
