// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_shader_atomic_counters GL_ARB_enhanced_layouts
// [end config]

#version 140
#extension GL_ARB_shader_atomic_counters: require
#extension GL_ARB_enhanced_layouts: require

const int start = 3;
layout(binding = 0, offset = start + 1) uniform atomic_uint x;

void main()
{
}
