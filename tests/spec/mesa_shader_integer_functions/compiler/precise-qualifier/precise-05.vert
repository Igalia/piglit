// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that a redeclaration of a built-in variable at global scope is allowed.

#version 130
#extension GL_MESA_shader_integer_functions: require

precise gl_Position;
