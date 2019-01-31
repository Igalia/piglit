// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_gpu_shader_fp64 GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "It is a compile-time error to use component 1 or 3 as the beginning of a
//   double or dvec2."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_gpu_shader_fp64: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 0, component = 3) out dvec2 b;

void main()
{
}
