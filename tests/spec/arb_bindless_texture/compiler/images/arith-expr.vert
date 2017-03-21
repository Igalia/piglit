// [config]
// expect_result: fail
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
//  "Images may not be implicitly converted to and from 64-bit integers,
//   and may not be used in arithmetic expressions."

uniform uvec2 u;

void main()
{
	writeonly image2D img;
	uvec2 r;

	r = img + u;
	r = img - u;
	r = img * u;
	r = img / u;
	r = img % u;
	img++;
	img--;
}
