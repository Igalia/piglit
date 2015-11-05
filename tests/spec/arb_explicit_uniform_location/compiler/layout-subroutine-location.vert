// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shader_subroutine GL_ARB_explicit_uniform_location GL_ARB_explicit_attrib_location
// [end config]

#version 150
#extension GL_ARB_shader_subroutine: require
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

subroutine void func_type();

/* A subroutine uniform for the above type */
layout(location = 2) subroutine uniform func_type f;
