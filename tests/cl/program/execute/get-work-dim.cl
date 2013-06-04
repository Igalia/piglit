/*!

[config]
name: get_work_dimensions
clc_version_min: 10

[test]
name: get_work_dim (1)
kernel_name: builtin_work_dim
dimensions: 1
global_size: 1 0 0
arg_out: 0 buffer int[1] 1

[test]
name: get_work_dim (2)
kernel_name: builtin_work_dim
dimensions: 2
global_size: 1 1 0
arg_out: 0 buffer int[1] 2

[test]
name: get_work_dim (3)
kernel_name: builtin_work_dim
dimensions: 3
global_size: 1 1 1
arg_out: 0 buffer int[1] 3

!*/

kernel void builtin_work_dim( __global int *ret ) {
  *ret = get_work_dim();
}

