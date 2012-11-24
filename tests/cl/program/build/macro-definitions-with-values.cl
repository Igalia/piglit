/*!
[config]
name: Macro definitions with values
clc_version_min: 10
build_options: -D BUILD_OPT1=1 -D BUILD_OPT2=2
!*/

kernel void dummy_kernel(global int* out){
    *out = BUILD_OPT1 + BUILD_OPT2;
}