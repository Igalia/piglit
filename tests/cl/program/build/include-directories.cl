/*!
[config]
name: Include Directories
clc_version_min: 10
build_options: -I . -I tests/cl/program/build
!*/

#include "include_test.h"

kernel void dummy_kernel(global int* out) {
    *out = BUILD_OPT;
}
