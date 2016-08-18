/*!
[config]
name: get_offset
clc_version_min: 10

kernel_name: fill

[test]
name: 1D, global_size 4 0 0, global_offset 9 0 0
dimensions: 1
global_size: 4 0 0
local_size: 4 0 0
global_offset: 9 0 0
arg_out: 0 buffer int[8] 9 9 10 9 11 9 12 9

[test]
name: 1D, global_size 4 0 0, global_offset 9 8 0
dimensions: 1
global_size: 4 0 0
local_size: 4 0 0
global_offset: 9 8 0
arg_out: 0 buffer int[8] 9 9 10 9 11 9 12 9

[test]
name: 1D, global_size 4 0 0, global_offset 9 8 7
dimensions: 1
global_size: 4 0 0
local_size: 4 0 0
global_offset: 9 8 7
arg_out: 0 buffer int[8] 9 9 10 9 11 9 12 9

[test]
name: 2D, global_size 4 4 0, global_offset 9 8 0
dimensions: 2
global_size: 4 4 0
local_size: 4 4 0
global_offset: 9 8 0
arg_out: 0 buffer int[32]  809 809  810 809  811 809  812 809 \
                           909 809  910 809  911 809  912 809 \
                          1009 809 1010 809 1011 809 1012 809 \
                          1109 809 1110 809 1111 809 1112 809

[test]
name: 2D, global_size 4 4 0, global_offset 9 8 7
dimensions: 2
global_size: 4 4 0
local_size: 4 4 0
global_offset: 9 8 7
arg_out: 0 buffer int[32]  809 809  810 809  811 809  812 809 \
                           909 809  910 809  911 809  912 809 \
                          1009 809 1010 809 1011 809 1012 809 \
                          1109 809 1110 809 1111 809 1112 809

[test]
name: 3D, global_size 4 4 4, global_offset 9 8 7
dimensions: 3
global_size: 4 4 4
local_size: 4 4 4
global_offset: 9 8 7
arg_out: 0 buffer int[128]  70809 70809  70810 70809  70811 70809  70812 70809 \
                            70909 70809  70910 70809  70911 70809  70912 70809 \
                            71009 70809  71010 70809  71011 70809  71012 70809 \
                            71109 70809  71110 70809  71111 70809  71112 70809 \
                            80809 70809  80810 70809  80811 70809  80812 70809 \
                            80909 70809  80910 70809  80911 70809  80912 70809 \
                            81009 70809  81010 70809  81011 70809  81012 70809 \
                            81109 70809  81110 70809  81111 70809  81112 70809 \
                            90809 70809  90810 70809  90811 70809  90812 70809 \
                            90909 70809  90910 70809  90911 70809  90912 70809 \
                            91009 70809  91010 70809  91011 70809  91012 70809 \
                            91109 70809  91110 70809  91111 70809  91112 70809 \
                           100809 70809 100810 70809 100811 70809 100812 70809 \
                           100909 70809 100910 70809 100911 70809 100912 70809 \
                           101009 70809 101010 70809 101011 70809 101012 70809 \
                           101109 70809 101110 70809 101111 70809 101112 70809

[test]
name: 3d, input dependent
kernel_name: dynamic
dimensions: 3
global_size: 1 1 1
local_size: 1 1 1
global_offset: 9 8 7
arg_in: 0 buffer int[5] 6 6 6 6 6
arg_out: 0 buffer int[5] 0 9 8 7 0
arg_in: 1 int 1
!*/

kernel void fill(global int* out) {
	unsigned int pos = get_local_id(0) + 4 * get_local_id(1) + 16 * get_local_id(2);
	unsigned int id = get_global_id(0) + 100*get_global_id(1) + 10000*get_global_id(2);
	unsigned int offset = get_global_offset(0) + 100 * get_global_offset(1) + 10000 * get_global_offset(2);
	out[2*pos] = id;
	out[2*pos+1] = offset;
}

kernel void dynamic(volatile global int *out, int j) {
	out[0] = get_global_offset(j-2);
	out[1] = get_global_offset(j-1);
	out[2] = get_global_offset(j);
	out[3] = get_global_offset(j+1);
	out[4] = get_global_offset(j+2);
}
