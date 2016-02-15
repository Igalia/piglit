// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_gpu_shader_fp64
// [end config]
//
// From the GL_ARB_enhanced_layouts spec:
//
//    "The offset must be a multiple of the size of the first component of the
//    first qualified variable or block member, or a compile-time error
//    results. Further, if applied to an aggregate containing a double, the
//    offset must also be a multiple of 8, and the space taken in the buffer
//    will be a multiple of 8."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_gpu_shader_fp64: require

struct S {
  float x;
  double y;
};

layout(xfb_offset = 8) out S s;

void main()
{
}
