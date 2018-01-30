// [config]
// expect_result: pass
// glsl_version: 1.50compatibility
// [end config]
#version 150 compatibility

mat3 func()
{
	return gl_NormalMatrix;
}
