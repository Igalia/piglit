/*!
[config]
name: Invalid CL Version Declaration
clc_version_min: 10
expect_build_fail: true
build_options: -cl-std=x.1
!*/

void dummy_function() {}
kernel void dummy_kernel() {
    dummy_function();
}