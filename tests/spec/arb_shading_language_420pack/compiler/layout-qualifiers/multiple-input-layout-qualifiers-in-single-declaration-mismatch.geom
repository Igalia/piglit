// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shading_language_420pack GL_ARB_gpu_shader5
// check_link: false
// [end config]
//
// From the ARB_gpu_shader5 spec:
//
//    "If an invocation count is declared, all such declarations must
//     specify the same count."
//
// From the ARB_shading_language_420pack spec:
//
//    "More than one layout qualifier may appear in a single declaration."

#version 150
#extension GL_ARB_shading_language_420pack: enable
#extension GL_ARB_gpu_shader5 : enable

layout(lines) layout(invocations=4) in;
layout(triangle_strip, max_vertices=3) out;

in vec4 Color1[];

uniform int foo[Color1.length() == 2 ? 1 : -1];

void main()
{
}
