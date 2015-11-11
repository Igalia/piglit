// [config]
// expect_result: fail
// glsl_version: 1.40
// check_link: true
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "Further, when location aliasing, the aliases sharing the location must
//   have the same underlying numerical type (floating-point or integer) and
//   the same auxiliary storage and interpolation qualification"

#version 140
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

// consume X/Y/Z components
layout(location = 0) out ivec3 a;

// consumes W component
layout(location = 0, component = 3) out uint b;

void main()
{
  a = ivec3(0);
  b = 1u;
}
