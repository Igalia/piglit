// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]
//
// Tests that compiler fails if a uniform is part of the constant expression.

#version 140
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_enhanced_layouts: require

uniform int n;

const int start = 3;
layout(location = n + start + 2) in vec4 b;

void main()
{
	gl_Position = b;
}
