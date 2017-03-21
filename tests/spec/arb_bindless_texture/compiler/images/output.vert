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
//  "Modify Section 4.3.6, Outputs, p. 36"
//
//  "(modify second paragraph, p. 37, to allow sampler and image outputs)
//   ... Output variables can only be floating-point scalars, floating-point
//   vectors, matrices, signed or unsigned integers or integer vectors,
//   sampler or image types, or arrays or structures of any these."

flat out writeonly image2D img;

void main()
{
}
