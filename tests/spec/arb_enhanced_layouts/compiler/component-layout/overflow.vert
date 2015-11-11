// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_separate_shader_objects
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "The components within a location are 0, 1, 2, and 3. A variable or
//   block member starting at component N will consume components N, N+1, N+2
//   , ... up through its size. It is a compile-time error if this sequence of
//   components gets larger than 3."

#version 140
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_separate_shader_objects: require

layout(location = 0, component = 4) out float a;

void main()
{
}
