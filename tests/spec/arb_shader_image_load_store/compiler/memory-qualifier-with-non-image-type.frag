// [config]
// expect_result: fail
// glsl_version: 3.30
// require_extensions: GL_ARB_shader_image_load_store
// [end config]

#version 330
#extension GL_ARB_shader_image_load_store: enable

// From Section 4.10 (Memory Qualifiers) of the GLSL 4.50 spec:
//
// "Variables declared as image types (the basic opaque types with “image”
//  in their keyword) can be further qualified with one or more of the
//  following memory qualifiers: ..."
//
// Easy enough to infer that memory qualifiers should not be used with
// non-image types.

uniform Block {
	volatile int x;
};

void main()
{
}
