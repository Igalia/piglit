// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "The offset must be a multiple of the size of the first component of the
//    first qualified variable or block member, or a compile-time error
//    results."

#version 150
#extension GL_ARB_enhanced_layouts: require

struct S {
  float x;
};

layout(xfb_offset = 2) out S s;

void main()
{
}
