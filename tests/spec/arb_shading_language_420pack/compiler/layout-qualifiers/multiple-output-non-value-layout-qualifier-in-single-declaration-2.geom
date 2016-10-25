// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_shading_language_420pack
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//    "More than one layout qualifier may appear in a single declaration."
//
// From section 4.3.8.2(Output Layout Qualifiers) of the GLSL 1.50 spec says:
//
//    "All geometry shader output layout declarations in a program must declare the
//     same layout and same value for max_vertices."

#version 150
#extension GL_ARB_shading_language_420pack: enable

layout(lines) in;
layout(max_vertices=3) layout(triangle_strip) out;
layout(points) out;

void main()
{
}
