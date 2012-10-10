/*!
[config]
name: Math Intrinsics
clc_version_min: 10
build_options: -cl-single-precision-constant -cl-denorms-are-zero
!*/

void dummy_function() {}

kernel void dummy_kernel() {
    dummy_function();
}