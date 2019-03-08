// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_separate_shader_objects GL_ARB_arrays_of_arrays
// check_link: true
// [end config]
//
// Test for explicit varying location overlap by arrays of arrays
#version 140
#extension GL_ARB_separate_shader_objects : require
#extension GL_ARB_arrays_of_arrays : require

uniform int i;

layout(location = 0) in vec4 out1[2][2];
layout(location = 3) in vec4 out2;

out vec4 color;

void main()
{
	if (i == 0)
		color = out1[0][0];
	else if (i == 1)
		color = out1[0][1];
	else if (i == 2)
		color = out1[1][0];
	else if (i == 3)
		color = out1[1][1];
	else
		color = out2;
}
