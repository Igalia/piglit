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
//  "These layouts may be specified at global scope to control the default
//   behavior of uniforms of the corresponding types, e.g."
//
//      layout (bindless_sampler) uniform;

layout (bound_image) uniform;

void main()
{
}
