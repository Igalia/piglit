// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "Only variables, block members, or blocks can be qualified with
//    *xfb_offset*."

#version 150
#extension GL_ARB_enhanced_layouts: require

layout (xfb_offset = 0) out;

void main()
{
}
