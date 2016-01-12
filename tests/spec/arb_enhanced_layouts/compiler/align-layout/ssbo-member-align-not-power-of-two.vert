// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_shader_storage_buffer_object
// check_link: false
// [end config]
//
// From Section 4.4.5 (Uniform and Shader Storage Block Layout Qualifiers) of
// the OpenGL 4.50 spec:
//
//   "The specified alignment must be a power of 2, or a compile-time error
//   results."
//

#version 140
#extension GL_ARB_enhanced_layouts : enable
#extension GL_ARB_shader_storage_buffer_object : enable

layout(std140) buffer b {
	layout(align = 28) vec4 var1;
	layout(align = 16) vec4 var2;
};

void main()
{
}
