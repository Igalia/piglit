// [config]
// expect_result: pass
// glsl_version: 1.10
// [end config]

#version 110

#ifdef this_is_undefined
#define VEC4_STRING_PARAM(a, b, c, d) vec4(#a, #b, c, d)
#endif

void main() { }
