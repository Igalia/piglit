// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_gpu_shader5
// check_link: false
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
// From the ARB_gpu_shader5 spec:
//
//    "If an invocation count is declared, all such declarations must
//     specify the same count."

#version 150
#extension GL_ARB_enhanced_layouts: enable
#extension GL_ARB_gpu_shader5 : enable

layout(points, invocations=4, invocations=3) in;
layout(invocations=4) in;
layout(triangle_strip, max_vertices=3) out;

void main()
{
}
