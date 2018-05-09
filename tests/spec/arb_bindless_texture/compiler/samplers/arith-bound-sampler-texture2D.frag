// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

uniform sampler2D tex;
uniform uvec2 handleOffset;

out vec4 finalColor;

// The ARB_bindless_texture spec says:
//
//  "Modify Section 5.4.1, Conversion and Scalar Constructors, p. 60"
//
//  "(add the following constructors:)"
//
//  "uvec2(any sampler type)     // Converts a sampler type to a
//                               //   pair of 32-bit unsigned integers
//   any sampler type(uvec2)     // Converts a pair of 32-bit unsigned integers to
//                               //   a sampler type"

void main()
{
	uvec2 handle = uvec2(tex);
	handle.x -= 0x12345678u;
	handle.y -= 0x9abcdef0u;
	finalColor = texture2D(sampler2D(handle + handleOffset), vec2(0, 0));
}
