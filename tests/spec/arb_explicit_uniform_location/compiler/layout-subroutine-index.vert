// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine GL_ARB_explicit_uniform_location
// [end config]

#version 150
#extension GL_ARB_shader_subroutine: require
#extension GL_ARB_explicit_uniform_location: require

subroutine void func_type();

/* A subroutine matching the above type */
layout(index = 2) subroutine (func_type) void f() {}
