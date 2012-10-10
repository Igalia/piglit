/*!
[config]
name: Macro Definitions
clc_version_min: 10
build_options: -D BUILD_OPT -D BUILD_OPT2
!*/

kernel void dummy_kernel(){
    int var1 = BUILD_OPT1; int var2 = BUILD_OPT2;
}