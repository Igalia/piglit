// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_tessellation_shader
// [end config]
//
// From the GLSL 4.5 spec:
//
// "It is a compile- or link-time error for the output vertex count to be less
// than or equal to zero"

#version 150
#extension GL_ARB_tessellation_shader: require
#extension GL_ARB_enhanced_layouts: require
layout(vertices = -1) out;

void main()
{
}
