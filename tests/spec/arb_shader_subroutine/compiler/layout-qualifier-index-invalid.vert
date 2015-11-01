// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine
// [end config]
//
// ARB_explicit_uniform_location spec allows a layout qualifier for
// subroutines, test for compile error when this is not available.

#version 150
#extension GL_ARB_shader_subroutine: require

subroutine void func_type();

/* A subroutine matching the above type */
layout(index = 2) subroutine (func_type) void f() {}
