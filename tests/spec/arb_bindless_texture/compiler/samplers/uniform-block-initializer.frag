// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

// The ARB_bindless_texture spec says:
//
//  "Replace Section 4.1.7 (Samplers), p. 25"
//
//  "As uniforms in the default uniform block, samplers may be initialized
//   only with the OpenGL API; they cannot be declared with an initializer
//   in a shader."

uniform sampler2D tex = 0;

void main()
{
}
