// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

layout (bindless_sampler) uniform sampler2D tex;
uniform uvec2 handleOffset;

out vec4 finalColor;

// The ARB_bindless_texture spec says:
//
//  "Replace Section 4.1.7 (Samplers), p. 25"
//
//   "Samplers can be used as l-values, so can be assigned into and used as
//   "out" and "inout" function parameters."

void adjustSamplerHandle(inout sampler2D s)
{
	uvec2 handle = uvec2(s);
	handle.x -= 0x12345678u;
	handle.y -= 0x9abcdef0u;
	s = sampler2D(handle + handleOffset);
}

void main()
{
	sampler2D s = tex;
	adjustSamplerHandle(s);
	finalColor = texture2D(s, vec2(0, 0));
}
