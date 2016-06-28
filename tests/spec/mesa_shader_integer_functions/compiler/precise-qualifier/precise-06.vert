// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that a straightforward redeclaration of a user-defined variable
// at global scope is allowed.

#version 130
#extension GL_MESA_shader_integer_functions: require

vec4 x;
precise x;
