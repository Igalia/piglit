// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_separate_shader_objects
// check_link: true
// [end config]
//
// Test for explicit varying location overlap by arrays
#version 140
#extension GL_ARB_separate_shader_objects : require

uniform int i;

layout(location = 0) in vec4 out1[2];
layout(location = 1) in vec4 out2;

out vec4 color;

void main()
{
	if (i == 0)
		color = out1[0];
	else if (i == 1)
		color = out1[1];
	else
		color = out2;
}
