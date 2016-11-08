// [config]
// expect_result: fail
// glsl_version: 3.00
// [end config]

// As a 64-bit integer, this is -1
#version 18446744073709551615 es

void main() { gl_Position = vec4(0); }
