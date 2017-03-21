// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable

// The ARB_bindless_texture spec says:
//
//  "Replace Section 4.1.X, (Images)"
//
//  "Images may be aggregated into arrays within a shader (using square
//   brackets []) and can be indexed with general integer expressions."

layout (bindless_image) uniform writeonly image2D imgs[64];

uniform vec4 color;
uniform uint a, b;

void main()
{
	imageStore(imgs[a], ivec2(0, 0), color);
	imageStore(imgs[a * b], ivec2(0, 0), color);
}
