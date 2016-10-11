// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_shading_language_420pack
// check_link: true
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//    "More than one layout qualifier may appear in a single declaration. If
//     the same layout-qualifier-name occurs in multiple layout qualifiers for
//     the same declaration, the last one overrides the former ones."
//
// From section 4.3.8.2(Output Layout Qualifiers) of the GLSL 1.50 spec says:
//
//    "All geometry shader output layout declarations in a program must declare the
//     same layout and same value for max_vertices."

#version 150
#extension GL_ARB_shading_language_420pack: enable

layout(lines) in;
layout(line_strip, max_vertices=2) layout(max_vertices=3) out;

in vec4 pos[];

layout(max_vertices=3) out;

void main()
{
}
