// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//   " No aliasing in output buffers is allowed: It is a compile-time
//     or link-time error to specify variables with overlapping
//     transform feedback offsets."

#version 140
#extension GL_ARB_enhanced_layouts: require

layout(xfb_offset = 0) out vec4 a;
layout(xfb_offset = 0) out vec4 b;

void main()
{
  a = vec4(1.0);
  b = vec4(0.0);
}
