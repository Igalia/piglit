// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// check_link: true
// [end config]
//
// From the ARB_enhanced_layouts spec:
//
//    "More than one layout qualifier may appear in a single declaration.
//     Additionally, the same layout-qualifier-name can occur multiple times
//     within a layout qualifier or across multiple layout qualifiers in the
//     same declaration. When the same layout-qualifier-name occurs
//     multiple times, in a single declaration, the last occurrence overrides
//     the former occurrence(s).  Further, if such a layout-qualifier-name
//     will effect subsequent declarations or other observable behavior, it
//     is only the last occurrence that will have any effect, behaving as if
//     the earlier occurrence(s) within the declaration are not present.
//     This is also true for overriding layout-qualifier-names, where one
//     overrides the other (e.g., row_major vs. column_major); only the last
//     occurrence has any effect."
//
// From section 4.3.8.2(Output Layout Qualifiers) of the GLSL 1.50 spec says:
//
//    "All geometry shader output layout declarations in a program must declare the
//     same layout and same value for max_vertices."

#version 150
#extension GL_ARB_enhanced_layouts: enable

layout(lines) in;
layout(line_strip, max_vertices=2, max_vertices=3) out;

in vec4 pos[];

layout(max_vertices=3) out;

void main()
{
}
