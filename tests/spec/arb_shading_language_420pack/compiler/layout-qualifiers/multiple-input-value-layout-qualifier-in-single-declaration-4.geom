// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shading_language_420pack GL_ARB_gpu_shader5
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//    "More than one layout qualifier may appear in a single declaration."
//
// From the ARB_gpu_shader5 spec:
//
//    "If an invocation count is declared, all such declarations must
//     specify the same count."

#version 150
#extension GL_ARB_shading_language_420pack: enable
#extension GL_ARB_gpu_shader5 : enable

layout(invocations=4) layout(lines) in;
layout(triangle_strip, max_vertices=3) out;
layout(invocations=3) in;

void main()
{
}
