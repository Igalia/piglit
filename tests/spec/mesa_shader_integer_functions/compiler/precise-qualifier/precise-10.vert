// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that precise redeclaration after the first use of a variable is not allowed.

#version 130
#extension GL_MESA_shader_integer_functions: require

float x;
x = 1;
precise x;	/* redeclaration after use */
