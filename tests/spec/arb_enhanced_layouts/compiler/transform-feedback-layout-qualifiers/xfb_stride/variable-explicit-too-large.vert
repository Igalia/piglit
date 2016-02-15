// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "The resulting stride (implicit or explicit) must be less than or equal
//    to the implementation-dependent constant
//    gl_MaxTransformFeedbackInterleavedComponents."

#version 150
#extension GL_ARB_enhanced_layouts: require

layout(xfb_stride = gl_MaxTransformFeedbackInterleavedComponents + 1) out vec4 var;

void main()
{
}
