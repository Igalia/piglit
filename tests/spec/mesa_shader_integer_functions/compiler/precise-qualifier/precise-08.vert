// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that `precise inout` is allowed on a function parameter

#version 130
#extension GL_MESA_shader_integer_functions: require

void foo(precise inout float x) {
	x += 1;
}
