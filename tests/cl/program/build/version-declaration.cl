/*!
[config]
name: CL Version Declaration
clc_version_min: 10
build_options: -cl-std=CL1.1
!*/

void dummy_function() {}

kernel void dummy_kernel() {
    dummy_function();
}