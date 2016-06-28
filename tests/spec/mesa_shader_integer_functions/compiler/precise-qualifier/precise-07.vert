// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// notable departure from the `invariant` qualifier rules: it seems reasonable
// to have local precise redeclarations be allowed.

#version 130
#extension GL_MESA_shader_integer_functions: require

void foo() {
	vec4 x;
	precise x;
}
