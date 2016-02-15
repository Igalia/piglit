// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "It is a compile-time error to specify an *xfb_buffer* that is greater
//    than the implementation-dependent constant
//    gl_MaxTransformFeedbackBuffers."

#version 150
#extension GL_ARB_enhanced_layouts: require

layout (xfb_buffer = gl_MaxTransformFeedbackBuffers - 1) out block {
  vec4 var;
};

void main()
{
}
