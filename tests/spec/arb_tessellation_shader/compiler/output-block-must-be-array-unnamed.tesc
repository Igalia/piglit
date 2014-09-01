// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// [end config]
//
// Per-vertex outputs from the tessellation control shader must be arrays.
// This test covers the case where an output array has no instance name,
// which is significant for Mesa's compiler.

#version 150
#extension GL_ARB_tessellation_shader: require

out block {
	vec4 x;
};	/* not an array */
