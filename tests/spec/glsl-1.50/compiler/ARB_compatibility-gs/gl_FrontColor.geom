// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// require_extensions: GL_ARB_compatibility
// [end config]
#version 150
#extension GL_ARB_compatibility : enable

void func()
{
	gl_FrontColor = gl_in[0].gl_FrontColor;
}
