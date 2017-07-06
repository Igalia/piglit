// [config]
// expect_result: fail
// glsl_version: 3.00
// [end config]
//
// Check that builtins may not be overloaded, even with different parameters.
//
// From GLSL ES 3.0 spec, chapter 6.1 "Function Definitions", page 71:
//
//     "A shader cannot redefine or overload built-in functions."
//
#version 300 es

int sqrt(int x) { return x; }
