// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "The *xfb_offset* qualifier assigns a byte offset within a transform
//    feedback buffer.  Only variables, block members, or blocks can be
//    qualified with *xfb_offset*."

#version 150
#extension GL_ARB_enhanced_layouts: require

layout(xfb_offset = 16) out block {
  vec4 var1;
  layout(xfb_offset = 0) vec4 var2;
} block_name;

void main()
{
}
