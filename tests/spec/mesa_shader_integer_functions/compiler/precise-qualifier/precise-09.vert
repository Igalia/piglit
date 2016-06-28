// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that precise redeclarations of function parameters are allowed.

#version 130
#extension GL_MESA_shader_integer_functions: require

void foo(out float x) {
	precise x;
	x = 1;
}
