// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// [end config]
//
// Inputs to the tessellation control shader must be arrays.

#version 150
#extension GL_ARB_tessellation_shader: require

in vec4 x;	/* not an array */
