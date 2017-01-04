// [config]
// expect_result: pass
// glsl_version: 3.30
// require_extensions: GL_ARB_explicit_attrib_location
// [end config]

#version 330
#extension GL_ARB_explicit_attrib_location: require
layout(location = 1) layout(location = 0) in vec4 vertex;

void main()
{
	gl_Position = vertex;
}
