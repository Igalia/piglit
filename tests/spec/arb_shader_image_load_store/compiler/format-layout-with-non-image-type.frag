// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_shader_image_load_store: enable

// From Section 4.4.6.2 (Format Layout Qualifiers) of the GLSL 4.50 spec:
//
// "Format layout qualifiers can be used on image variable declarations
//  (those declared with a basic type having “image” in its keyword)."
//
// Easy enough to infer that format layout qualifiers should not be used with
// non-image types.

uniform Block {
	layout (r32f) int x;
};

void main()
{
}
