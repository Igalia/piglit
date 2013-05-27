/*!
[config]
name: switch_case_result
clc_version_min: 10
kernel_name: switch_case

[test]
name: switch_case
kernel_name: switch_case
dimensions: 1
global_size: 16 0 0
local_size: 16 0 0
arg_out: 0 buffer int[16] 4 15 15 11 12 13 17 17 16 17 19 19 18 21 22 23
!*/

__kernel void switch_case(__global int *out)
{
	switch (get_global_id(0)) {
		case 0: out[get_global_id(0)] = get_global_id(0) + 4;
			break;
		case 1: out[get_global_id(0)] = get_global_id(0) + 14;
			break;
		case 2: out[get_global_id(0)] = get_global_id(0) + 13;
			break;
		case 6: out[get_global_id(0)] = get_global_id(0) + 11;
			break;
		case 7: out[get_global_id(0)] = get_global_id(0) + 10;
			break;
		case 10: out[get_global_id(0)] = get_global_id(0) + 9;
			 break;
		case 12: out[get_global_id(0)] = get_global_id(0) + 6;
			 break;
		default: out[get_global_id(0)] = get_global_id(0) + 8;
			 break;
	}
}

