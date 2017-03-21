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
//  "(modify last paragraph, p. 35, allowing samplers and images as fragment
//   shader inputs) ... Fragment inputs can only be signed and unsigned
//   integers and integer vectors, floating point scalars, floating-point
//   vectors, matrices, sampler and image types, or arrays or structures of
//   these.  Fragment shader inputs that are signed or unsigned integers,
//   integer vectors, or any double-precision floating- point type, or any
//   sampler or image type must be qualified with the interpolation qualifier
//   "flat"."

flat in writeonly image2D sampler;

void main()
{
}
