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
// From section 4.3.8.1 (Input Layout Qualifiers) of the GLSL 1.50 spec says:
//
//    "At least one geometry shader (compilation unit) in a program
//     must declare an input layout, and all geometry shader input
//     layout declarations in a program must declare the same
//     layout. It is not required that all geometry shaders in a
//     program declare an input layout."

#version 150
#extension GL_ARB_enhanced_layouts: enable

layout(triangles, lines) in;
layout(lines) in;
layout(line_strip, max_vertices=3) out;

in vec4 pos[];

void main()
{
}
