// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// Test that overloads which differ only by return type are not allowed.

#version 130
#extension GL_MESA_shader_integer_functions : enable

int foo(int x) { return 0; }
float foo(int x) { return 0; }		/* differs only by return type. */
