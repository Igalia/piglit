// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_separate_shader_objects
// check_link: false
// [end config]
//
// From the ARB_shading_language_420pack spec:
//
//   "More than one layout qualifier may appear in a single declaration. If
//   the same layout-qualifier-name occurs in multiple layout qualifiers for
//   the same declaration, the last one overrides the former ones."

#version 140
#extension GL_ARB_separate_shader_objects : enable

layout(location=2) layout(location=1) out vec3 var;

void main()
{
}
