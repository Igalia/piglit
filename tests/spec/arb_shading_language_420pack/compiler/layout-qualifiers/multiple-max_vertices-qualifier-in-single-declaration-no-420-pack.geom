// [config]
// expect_result: fail
// glsl_version: 1.50
// check_link: false
// [end config]
//
// Section 4.3.8.2(Output Layout Qualifiers) of the GLSL 1.50 spec says:
//
//   "All geometry shader output layout declarations in a program must
//    declare the same layout and same value for max_vertices."
//
// but ...
//
// From the ARB_shading_language_420pack spec:
//
//   "More than one layout qualifier may appear in a single
//    declaration. If the same layout-qualifier-name occurs in
//    multiple layout qualifiers for the same declaration, the last
//    one overrides the former ones."

#version 150

layout(lines) in;
layout(line_strip, max_vertices=3) layout(max_vertices=2) out;

void main()
{
}
