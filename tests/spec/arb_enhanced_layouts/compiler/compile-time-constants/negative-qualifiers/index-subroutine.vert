// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine GL_ARB_enhanced_layouts GL_ARB_explicit_uniform_location
// [end config]
//
// From the ARB_explicit_uniform_location spec:
//
// "The subroutine functions can be assigned an explicit index in the shader
// between 0 and MAX_SUBROUTINES minus one."

#version 150
#extension GL_ARB_shader_subroutine: require
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_uniform_location: require

const int start = -3;
subroutine void func_type();

/* A subroutine matching the above type */
layout(index = start + 2) subroutine (func_type) void f() {}
