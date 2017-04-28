// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable

struct foo {
	float x;
	writeonly image2D img;
};

uniform Block {
   foo f;
};

void main()
{
	imageStore(f.img, ivec2(0, 0), vec4(1, 2, 3, 4));
}
