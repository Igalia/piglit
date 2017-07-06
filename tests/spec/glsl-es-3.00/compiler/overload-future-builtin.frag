// [config]
// expect_result: pass
// glsl_version: 3.00
// [end config]
//
// Check that builtins that are not available in ESSL 3.00 may be defined
// or overloaded as needed.
//
#version 300 es
precision highp float;

int bitfieldExtract(int x) { return x; }
int imageAtomicAdd(int x) { return x; }
void barrier() { }
uint packUnorm4x8(vec4 v) { return uint(v.x); }
