// [config]
// expect_result: pass
// glsl_version: 1.10
// require_extensions: GL_ARB_explicit_uniform_location
// [end config]

#version 110
#extension GL_ARB_explicit_uniform_location: require
vec4 vertex;
layout(location = 41) uniform float foo;

void main()
{
	gl_Position = vertex;
}
