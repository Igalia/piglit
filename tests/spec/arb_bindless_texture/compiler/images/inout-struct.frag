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
//  "Images can be used as l-values, so can be assigned into and used as
//   "out" and "inout" function parameters."

struct foo {
	float x;
	writeonly image2D img;
};

void f(inout foo p)
{
}

void main()
{
}
