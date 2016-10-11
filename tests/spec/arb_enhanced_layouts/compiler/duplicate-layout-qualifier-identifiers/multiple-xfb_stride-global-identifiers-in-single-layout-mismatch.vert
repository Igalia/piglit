// [config]
// expect_result: fail
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
// From the ARB_enhanced_layouts spec:
//
//    "While *xfb_stride* can be declared multiple times for the same buffer,
//     it is a compile-time or link-time error to have different values
//     specified for the stride for the same buffer."

#version 150
#extension GL_ARB_enhanced_layouts: require

layout(xfb_stride = 32, xfb_stride = 20) out;
layout(xfb_buffer = 0, xfb_stride = 32) out;

void main()
{
}
