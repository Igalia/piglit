// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// [end config]
//
// Per-vertex inputs to the tessellation evaluation shader must be arrays.

#version 150
#extension GL_ARB_tessellation_shader: require

in block {
	vec4 x;
} xs;	/* not an array */
