// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable

writeonly uniform image2D img;
uniform uvec2 handleOffset;

out vec4 finalColor;

// The ARB_bindless_texture spec says:
//
//  "Modify Section 5.4.1, Conversion and Scalar Constructors, p. 60"
//
//  "(add the following constructors:)"
//
//  "uvec2(any image type)       // Converts an image type to a
//                               //   pair of 32-bit unsigned integers
//   any image type(uvec2)       // Converts a pair of 32-bit unsigned integers to
//                               //   an image type"

void main()
{
	uvec2 handle = uvec2(img);
	handle.x -= 0x12345678u;
	handle.y -= 0x9abcdef0u;
	imageStore(image2D(handle + handleOffset), ivec2(0, 0), vec4(1, 2, 3, 4));
}
