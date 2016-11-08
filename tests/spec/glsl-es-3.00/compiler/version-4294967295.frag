// [config]
// expect_result: fail
// glsl_version: 3.00
// [end config]

// As a 32-bit integer, this is -1
#version 4294967295 es

void main() { gl_FragColor = vec4(0); }
