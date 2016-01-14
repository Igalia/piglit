// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader5 GL_ARB_shading_language_420pack
// check_link: false
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//   "More than one layout qualifier may appear in a single declaration. If
//   the same layout-qualifier-name occurs in multiple layout qualifiers for
//   the same declaration, the last one overrides the former ones."

#version 150
#extension GL_ARB_gpu_shader5 : enable
#extension GL_ARB_shading_language_420pack: enable

layout(points) in;
layout(points) out;

layout(stream=2) layout(stream=1) out vec3 var;

void main()
{
}
