// [config]
// expect_result: fail
// glsl_version: 1.10
// [end config]

// As a 64-bit integer, this is -1
#version 18446744073709551615

void main() { gl_FragColor = vec4(0); }
