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
//  "Replace Section 4.1.X, (Images)"
//
//  "As uniforms in the default uniform block, images may be initialized
//   only with the OpenGL API; they cannot be declared with an initializer
//   in a shader."

writeonly uniform image2D img = 0;

void main()
{
}
