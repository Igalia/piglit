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
//  "Samplers may not be implicitly converted to and from 64-bit integers,
//   and may not be used in arithmetic expressions."

void main()
{
	sampler2D tex = uvec2(0, 0);
	uvec2 pair = tex;
}
