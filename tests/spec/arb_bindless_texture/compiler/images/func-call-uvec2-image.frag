// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable

layout (bindless_image) writeonly uniform image2D img;
uniform uvec2 handleOffset;

out vec4 finalColor;

// The ARB_bindless_texture spec says:
//
//  "Replace Section 4.1.7 (Samplers), p. 25"
//
//   "Samplers can be used as l-values, so can be assigned into and used as
//   "out" and "inout" function parameters."

void adjustImageHandle(inout writeonly image2D img)
{
	uvec2 handle = uvec2(img);
	handle.x -= 0x12345678u;
	handle.y -= 0x9abcdef0u;
	img = image2D(handle + handleOffset);
}

void main()
{
	writeonly image2D _img = img;
	adjustImageHandle(_img);
	imageStore(_img, ivec2(0, 0), vec4(1, 2, 3, 4));
}
