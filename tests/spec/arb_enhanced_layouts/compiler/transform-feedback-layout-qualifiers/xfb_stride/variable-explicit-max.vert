// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "The resulting stride (implicit or explicit) must be less than or equal
//    to the implementation-dependent constant
//    gl_MaxTransformFeedbackInterleavedComponents."

#version 140
#extension GL_ARB_enhanced_layouts: require

layout(xfb_stride = gl_MaxTransformFeedbackInterleavedComponents) out vec4 var;

void main()
{
}
