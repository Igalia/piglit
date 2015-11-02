// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_shader_storage_buffer_object GL_ARB_enhanced_layouts
// [end config]
//
// From Section 4.4.5 (Uniform and Shader Storage Block Layout Qualifiers)
// of the GLSL 4.5 spec:
//
// "If the binding point for any uniform or shader storage block instance is
// less than zero, or greater than or equal to the implementation-dependent
// maximum number of uniform buffer bindings, a compile-time error will occur."

#version 140
#extension GL_ARB_shader_storage_buffer_object: require
#extension GL_ARB_enhanced_layouts: require

const int start = 3;
layout(binding = start - 4) buffer buf {
	float f;
};

float foo(void) {
	return f;
}
