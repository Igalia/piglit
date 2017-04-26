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
//  "As function parameters, images may be only passed to images of
//   matching type."

void f(inout image2D p)
{
}

void main()
{
	writeonly image1D u;
	f(u);
}
