// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "The offset must be a multiple of the size of the first component of the
//    first qualified variable or block member, or a compile-time error
//    results."

#version 140
#extension GL_ARB_enhanced_layouts: require

layout(xfb_offset = 2) out vec4 var;

void main()
{
}
