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
//  "Modify Section 4.4.6 Opaque-Uniform Layout Qualifiers of the GLSL 4.30
//   spec"
//
//  "They may also be specified on a uniform variable declaration of a
//   corresponding type, e.g."
//
//      layout (bindless_sampler) uniform sampler2D mySampler;

layout (bindless_image) writeonly uniform image2D img;

void main()
{
}
