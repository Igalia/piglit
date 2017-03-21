// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// check_link: true
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

// The ARB_bindless_texture spec says:
//
//  "If both bindless_sampler and bound_sampler, or bindless_image and
//   bound_image, are declared at global scope in any compilation unit, a link-
//   time error will be generated. In the absence of these qualifiers, sampler
//   and image uniforms are considered "bound".  Additionally, if
//   GL_ARB_bindless_texture is not enabled, these uniforms are considered
//   "bound"."

layout (bindless_sampler) uniform;
layout (bound_sampler) uniform;

void main()
{
}
