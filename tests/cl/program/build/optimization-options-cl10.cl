/*!
[config]
name: Optimization Options
clc_version_min: 10
clc_version_max: 10
build_options: -cl-opt-disable -cl-strict-aliasing -cl-mad-enable -cl-finite-math-only -cl-no-signed-zeros -cl-unsafe-math-optimizations -cl-fast-relaxed-math
!*/

void dummy_function() {}

kernel void dummy_kernel(global int* out) {
    dummy_function();
}