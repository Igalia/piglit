// [config]
// expect_result: fail
// glsl_version: 1.40
// check_link: true
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "It is a compile-time or link-time error to have any *xfb_offset*
//    that overflows *xfb_stride*, whether stated on declarations before or
//    after the *xfb_stride*, or in different compilation units."

#version 140
#extension GL_ARB_enhanced_layouts: require

layout(xfb_stride = 20) out vec4 var;
layout(xfb_offset = 16) out vec4 var2;

void main()
{
  var = vec4(1.0);
  var2 = vec4(0.0);
}
