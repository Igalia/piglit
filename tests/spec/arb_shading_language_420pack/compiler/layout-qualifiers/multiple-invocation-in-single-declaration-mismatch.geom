// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shading_language_420pack
// check_link: false
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//    "More than one layout qualifier may appear in a single declaration. If
//     the same layout-qualifier-name occurs in multiple layout qualifiers for
//     the same declaration, the last one overrides the former ones."
//
// From the ARB_gpu_shader5 spec:
//
//    "If an invocation count is declared, all such declarations must
//     specify the same count."

#version 150
#extension GL_ARB_shading_language_420pack: enable
#extension GL_ARB_gpu_shader5 : enable

#version 150

layout(points, invocations=4) layout(invocations=3) in;
layout(invocations=4) in;
layout(triangle_strip, max_vertices=3) out;

void main()
{
}
