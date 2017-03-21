// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

// The ARB_bindless_texture spec says:
//
//  "Replace Section 4.1.7 (Samplers), p. 25"
//
//  "Samplers can be used as l-values, so can be assigned into and used as
//   "out" and "inout" function parameters."

struct foo {
	float x;
	sampler2D tex;
};

uniform foo u;

foo f()
{
	return u;
}

void main()
{
}
