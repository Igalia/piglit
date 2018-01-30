// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// [end config]
#version 150 compatibility

void main()
{
	gl_TexCoord[0] = gl_in[0].gl_TexCoord[0];
}
