// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_separate_shader_objects GL_ARB_enhanced_layouts
// check_link: false
// [end config]
//
// From the ARB_enhanced_layouts spec:
//
//   "More than one layout qualifier may appear in a single declaration.
//   Additionally, the same layout-qualifier-name can occur multiple times
//   within a layout qualifier or across multiple layout qualifiers in the
//   same declaration"

#version 140
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_enhanced_layouts: enable

layout(location=2, location=1) out vec3 var;

void main()
{
}
