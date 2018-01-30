// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// [end config]
#version 150 compatibility

void func()
{
	gl_FogFragCoord = gl_in[0].gl_FogFragCoord;
}
