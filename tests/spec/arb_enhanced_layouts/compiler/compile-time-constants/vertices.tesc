// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_tessellation_shader
// [end config]

#version 150
#extension GL_ARB_tessellation_shader: require
#extension GL_ARB_enhanced_layouts: require

const int start = 3;
layout(vertices = start - 1) out;

void main()
{
}
