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
//  "Samplers aggregated into arrays within a shader (using square
//   brackets []) can be indexed with arbitrary integer expressions."

layout (bindless_sampler) uniform sampler2D texs[64];

uniform uint a, b;
out vec4 color;

void main()
{
	color  = texture2D(texs[a], vec2(0, 0));
	color += texture2D(texs[a * b], vec2(0, 0));
}
