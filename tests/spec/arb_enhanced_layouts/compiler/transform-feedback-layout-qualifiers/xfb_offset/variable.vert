// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "The *xfb_offset* qualifier assigns a byte offset within a transform
//    feedback buffer.  Only variables, block members, or blocks can be
//    qualified with *xfb_offset*."

#version 140
#extension GL_ARB_enhanced_layouts: require

layout(xfb_offset = 16) out vec4 var1;

void main()
{
}
