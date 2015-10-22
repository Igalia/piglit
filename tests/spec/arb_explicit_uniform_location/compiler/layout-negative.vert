// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_ARB_explicit_attrib_location GL_ARB_explicit_uniform_location
// [end config]
//
// From the ARB_explicit_uniform_location spec:
//
// "Valid locations for default-block uniform variable locations are in the
// range of 0 to the implementation-defined maximum number of uniform
// locations."

#version 130
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_explicit_uniform_location: require
vec4 vertex;
layout(location = -1) uniform float foo;

void main()
{
	gl_Position = vertex;
}
