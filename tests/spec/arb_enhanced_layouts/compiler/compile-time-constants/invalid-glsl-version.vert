// [config]
// expect_result: fail
// glsl_version: 1.40
// [end config]
//
// Test that compiler fails when compile-time constants used in unsupported
// version of glsl and ARB_enhanced_layouts is not enabled.

#version 140
#extension GL_ARB_explicit_attrib_location: require

const int start = 3;
layout(location = start + 2) in vec4 b;

void main()
{
	gl_Position = b;
}
