// [config]
// expect_result: pass
// glsl_version: 1.20
// require_extensions: GL_ARB_shader_storage_buffer_object
// [end config]

#version 120
#extension GL_ARB_shader_storage_buffer_object: require

// From Section 4.10 (Memory Qualifiers) of the GLSL 4.50 spec:
//
// "The memory qualifiers coherent, volatile, restrict, readonly, and
//  writeonly may be used in the declaration of buffer variables
//  (i.e., members of shader storage blocks)"

buffer ssbo {
	readonly int a;
	writeonly int b;
	coherent int c;
	volatile int d;
	restrict int e;
};
