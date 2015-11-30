// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//    "It is a compile-time error to use a location qualifier on a member of
//    a structure."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 0) in struct S {
  vec4 a;
  float f1;
  layout(location=1) float f2; // Error: can't use location on struct member
  float f3;
} s;

float foo(void) {
  return s.f1 + s.f2 + s.f3 + s.a.x + s.a.y + s.a.z + s.a.w;
}
