// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]

// As a 32-bit integer, this is -1
#version 4294967295

void main() { gl_Position = vec4(0); }
