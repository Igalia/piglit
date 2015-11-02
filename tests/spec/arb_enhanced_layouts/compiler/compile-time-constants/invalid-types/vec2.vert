// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]
//
// Tests that only integral constant expression can be used.
//
// Section 4.3.3 (Constant Expressions) from the GLSL 4.50 spec:
// "An integral constant expression is a constant expression that evaluates to
//  a scalar signed or unsigned integer."

#version 140
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_enhanced_layouts: require

const vec2 start = vec2(1.0);
layout(location = start) in vec4 b;

void main()
{
	gl_Position = b;
}
