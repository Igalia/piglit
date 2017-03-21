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
//  "Modify Section 4.3.6, Outputs, p. 36"
//
//  "(do not modify the last paragraph, p. 38; samplers and images are not
//   allowed as fragment shader outputs)"

out writeonly image2D image;

void main()
{
}
