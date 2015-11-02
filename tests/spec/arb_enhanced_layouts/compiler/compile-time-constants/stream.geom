// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader5 GL_ARB_enhanced_layouts
// check_link: false
// [end config]

#version 150
#extension GL_ARB_gpu_shader5: enable
#extension GL_ARB_enhanced_layouts: require

const int start = 1;
layout(points) in;
layout(points, stream = start + 2, max_vertices = 3) out;
out vec4 streamout;

void main()
{
}
