// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_gpu_shader_fp64 GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "A variable or block member starting at component N will consume
//   components N, N+1, N+2, ... up through its size. It is a compile-time
//   error if this sequence of components gets larger than 3. A scalar double
//   will consume two of these components, and a dvec2 will consume all four
//   components available within a location."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_gpu_shader_fp64: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 0, component = 2) out dvec2 b;

void main()
{
}
