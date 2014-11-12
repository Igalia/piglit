/*!
[config]
name: Optimization Options CL1.1+
clc_version_min: 11
build_options: -cl-opt-disable -cl-mad-enable -cl-finite-math-only -cl-no-signed-zeros -cl-unsafe-math-optimizations -cl-fast-relaxed-math
!*/

void dummy_function() {}

kernel void dummy_kernel(global int* out) {
    dummy_function();
}