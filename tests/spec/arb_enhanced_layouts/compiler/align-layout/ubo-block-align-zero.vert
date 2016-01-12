// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// From Section 4.4.5 (Uniform and Shader Storage Block Layout Qualifiers) of
// the OpenGL 4.50 spec:
//
//   "The specified alignment must be a power of 2, or a compile-time error
//   results."

#version 140
#extension GL_ARB_enhanced_layouts : enable

layout(std140, align = 0) uniform block {
	vec4 var1;
	vec4 var2;
};

void main()
{
}
