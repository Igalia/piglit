// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_gpu_shader_fp64 GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "A dvec3 or dvec4 can only be declared without specifying a component."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_gpu_shader_fp64: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 0, component = 1) out dvec3 b;

void main()
{
}
