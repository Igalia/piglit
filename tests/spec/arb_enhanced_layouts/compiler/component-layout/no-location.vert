// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "It is a compile-time error to use component without also specifying the
//    location qualifier (order does not matter)."

#version 140
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_attrib_location: require

layout(location = 0) in vec3 a;

// no location
layout(component = 3) in float b;

void main()
{
  gl_Position = vec4(a, b);
}
