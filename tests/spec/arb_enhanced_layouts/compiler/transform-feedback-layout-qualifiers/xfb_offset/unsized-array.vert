// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "It is a compile-time error to apply xfb_offset to the declaration of an
//    unsized array."

#version 140
#extension GL_ARB_enhanced_layouts: require

layout(xfb_offset = 0) out vec4 var[];

void main()
{
}
