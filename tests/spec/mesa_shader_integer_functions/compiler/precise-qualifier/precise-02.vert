// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that `precise` is allowed on shader outputs.

#version 130
#extension GL_MESA_shader_integer_functions: require

precise out vec4 x;
