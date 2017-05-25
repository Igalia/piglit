// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require
#extension GL_ARB_shader_image_load_store: enable
#extension GL_ARB_arrays_of_arrays: enable

// From Section 4.4.6.2 (Format Layout Qualifiers) of the GLSL 4.50 spec:
//
// "Uniforms not qualified with writeonly must have a format layout qualifier."
//
// Because GL_ARB_bindless_texture allows to declare images insides structures,
// this rule applies and this test should fail.

struct S_inner {
	image2D img;
};

struct {
	S_inner si[2][2];
} s;

void main()
{
}
