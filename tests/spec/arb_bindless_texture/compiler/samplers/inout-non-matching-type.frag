// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

// The ARB_bindless_texture spec says:
//
//  "Replace Section 4.1.7 (Samplers), p. 25"
//
//  "As function parameters, samplers may be only passed to samplers of
//   matching type."

void f(inout sampler2D p)
{
}

void main()
{
	sampler1D u;
	f(u);
}
