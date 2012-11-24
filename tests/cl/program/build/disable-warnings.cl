/*!
[config]
name: Disable Warnings
clc_version_min: 10
build_options: -w
!*/

void dummy_function() {
    int i = 0; //unused variable
}

kernel void dummy_kernel(global int* out) {
    dummy_function();
}