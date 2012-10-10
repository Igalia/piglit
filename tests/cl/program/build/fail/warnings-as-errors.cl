/*!
[config]
name: Treat warnings as errors
clc_version_min: 10
build_options: -Werror
expect_build_fail: true
!*/

void dummy_function() { int i = 0; }

kernel void dummy_kernel() {
    dummy_function();
}