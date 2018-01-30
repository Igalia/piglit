// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// [end config]
#version 150 compatibility

vec4 func()
{
	return gl_Fog.color;
}
