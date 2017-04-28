// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable
#extension GL_ARB_arrays_of_arrays: enable

struct s {
	writeonly image2D img[3][2];
	int y;
};

void main()
{
	s a[2][4];
	imageStore(a[0][0].img[0][0], ivec2(0, 0), vec4(1, 2, 3, 4));
}
