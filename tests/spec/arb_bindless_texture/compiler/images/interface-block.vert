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
//  "Modify Section 4.3.7, Interface Blocks, p. 38"
//
//  "(remove the following bullet from the last list on p. 39, thereby
//   permitting sampler types in interface blocks; image types are also
//   permitted in blocks by this extension)"
//
//      * sampler types are not allowed

uniform Images {
	writeonly image2D image;
} images;

void main()
{
}
