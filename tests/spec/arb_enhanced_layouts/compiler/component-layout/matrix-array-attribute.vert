// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]
//
// From Section 4.4.1 (Input Layout Qualifiers) of the GLSL 4.50 spec:
//
//   "It is a compile-time error to apply the component qualifier to a matrix,
//   a structure, a block, or an array containing any of these."

#version 150
#extension GL_ARB_enhanced_layouts: require
#extension GL_ARB_explicit_attrib_location: require

layout(location = 0, component = 1) in mat3 a[32];

void main()
{
}
