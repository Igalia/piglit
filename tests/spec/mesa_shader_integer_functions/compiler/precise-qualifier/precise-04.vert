// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that `precise` is allowed as a parameter qualifier.

#version 130
#extension GL_MESA_shader_integer_functions: require

void foo(precise out float x) {
	x = 1;
}
