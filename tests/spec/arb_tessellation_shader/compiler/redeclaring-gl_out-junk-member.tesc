// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_tessellation_shader
// [end config]

#version 150
#extension GL_ARB_tessellation_shader: require

out gl_PerVertex {
	vec4 gl_Position;
	vec4 junk;	/* must be subset of implicit decl! */
} gl_out[];
