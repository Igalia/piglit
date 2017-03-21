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
//  "Samplers may be declared as shader inputs and outputs, as uniform
//   variables, as temporary variables, and as function parameters."

void main()
{
	sampler2D tex;
}
