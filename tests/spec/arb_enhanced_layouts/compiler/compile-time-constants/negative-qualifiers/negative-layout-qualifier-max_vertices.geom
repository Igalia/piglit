// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]

#version 150
#extension GL_ARB_enhanced_layouts: require

layout(triangles) in;
layout(triangle_strip, max_vertices = -1) out;

void main()
{
}
