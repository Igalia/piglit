/*!
[config]
name: Include Directories
clc_version_min: 10
build_options: -I . -I tests/cl/program/build
!*/

#include "include_test.h"

void dummy_function() {}

kernel void dummy_kernel() {
    dummy_function();
    int test = BUILD_OPT;
}
