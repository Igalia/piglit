// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_tessellation_shader
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
// From the ARB_tessellation_shader spec:
//
//    "All tessellation control shader layout declarations in a program must
//     specify the same output patch vertex count."

#version 150
#extension GL_ARB_enhanced_layouts: enable
#extension GL_ARB_tessellation_shader: require

layout(vertices = 3, vertices = 4) out;
layout(vertices = 3) out;

void main() {
    gl_out[gl_InvocationID].gl_Position = vec4(0.0);
    gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 1.0);
    gl_TessLevelInner = float[2](1.0, 1.0);
}
