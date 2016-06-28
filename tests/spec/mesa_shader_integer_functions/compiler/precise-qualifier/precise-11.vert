// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// test that a precise redeclaration of a parameter after use is not allowed.

#version 130
#extension GL_MESA_shader_integer_functions: require

void foo(out x) {
	x = 1;
	precise x;	/* redeclaration after use */
}
