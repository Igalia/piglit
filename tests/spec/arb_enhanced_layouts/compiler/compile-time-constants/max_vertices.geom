// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]

#version 150
#extension GL_ARB_enhanced_layouts: require

const int start = 3;
const int offset = 1;
layout(triangles) in;
layout(triangle_strip, max_vertices = start - offset) out;

void main()
{
}
