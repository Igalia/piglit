// [config]
// expect_result: pass
// glsl_version: 1.40compatibility
// require_extensions: GL_ARB_compatibility
// [end config]
#version 140
#extension GL_ARB_compatibility : enable

vec4 func()
{
	return gl_ObjectPlaneS[0];
}
