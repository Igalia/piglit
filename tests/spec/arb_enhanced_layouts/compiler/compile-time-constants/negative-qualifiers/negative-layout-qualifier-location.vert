// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts GL_ARB_explicit_attrib_location
// [end config]
//
// From the ARB_explicit_uniform_location spec:
//
// "Valid locations for default-block uniform variable locations are in the
// range of 0 to the implementation-defined maximum number of uniform
// locations."

#version 140
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_enhanced_layouts: require

layout(location = -1) in vec4 attrb;

void main()
{
}
