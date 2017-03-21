// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_bindless_texture
// [end config]

#version 330
#extension GL_ARB_bindless_texture: require

// The ARB_bindless_texture spec says:
//
//  "Modify Section 4.3.7, Interface Blocks, p. 38"
//
//  "(remove the following bullet from the last list on p. 39, thereby
//   permitting sampler types in interface blocks; image types are also
//   permitted in blocks by this extension)"
//
//      * sampler types are not allowed

uniform Samplers {
	sampler2D tex;
} samplers;

void main()
{
}
