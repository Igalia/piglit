// [config]
// expect_result: fail
// glsl_version: 1.20
// require_extensions: GL_ARB_uniform_buffer_object GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_uniform_buffer_object: require
#extension GL_ARB_shader_storage_buffer_object: enable

// From Section 4.10 (Memory Qualifiers) of the GLSL 4.50 spec:
//
// "Memory qualifiers are only supported in the declarations of image
//  variables, buffer variables, and shader  storage blocks; it is an error to
//  use such qualifiers in any other declarations."

uniform Block {
	readonly int a;
	writeonly int b;
	coherent int c;
	volatile int d;
	restrict int e;
};

void main()
{
}
