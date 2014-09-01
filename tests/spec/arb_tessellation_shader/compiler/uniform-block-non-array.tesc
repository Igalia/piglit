// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// [end config]
//
// Uniform blocks should not be affected by array requirements for input or output
// blocks.

#version 150
#extension GL_ARB_tessellation_shader: require

uniform block {
	vec4 x;
} xs;	/* not an array, should work */
