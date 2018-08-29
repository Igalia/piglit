// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]

#version 110

#define VEC4_STRING_PARAM(a, b, c, d) vec4(#a, #b, c, d)

void main() { }
