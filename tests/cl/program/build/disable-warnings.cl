/*!
[config]
name: Disable Warnings
clc_version_min: 10
build_options: -w
!*/

void dummy_function() { int i = 0; }

kernel void dummy_kernel() {
    dummy_function();
}