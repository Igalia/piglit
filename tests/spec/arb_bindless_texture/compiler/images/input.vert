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
//  "Modify Section 4.3.4, Inputs, p. 34"
//
//  "(modify third paragraph of the section to allow sampler and image types)
//   ...  Vertex shader inputs can only be float, single-precision
//   floating-point scalars, single-precision floating-point vectors,
//   matrices, signed and unsigned integers and integer vectors, sampler and
//   image types."

in writeonly image2D image;

void main()
{
}
