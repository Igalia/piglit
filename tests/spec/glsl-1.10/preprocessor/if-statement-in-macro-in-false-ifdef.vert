// [config]
// expect_result: pass
// glsl_version: 1.10
// [end config]
//
// Reproduces https://bugs.freedesktop.org/show_bug.cgi?id=107772

#version 110

#ifdef NOT_DEFINED
#define A_MACRO(x) \
	if (x)
#endif

void main()
{
    gl_Position = vec4(0);
}
