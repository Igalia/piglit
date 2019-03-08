// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_separate_shader_objects
// check_link: true
// [end config]
//
// Test for explicit varying location overlap by structs
#version 150
#extension GL_ARB_separate_shader_objects : require

uniform int i;

layout(location = 0) in struct S {
	vec4 out1;
	vec4 out2;
} s;

layout(location = 1) in vec4 out3;

out vec4 color;

void main()
{
	if (i == 0)
		color = s.out1;
	else if (i == 1)
		color = s.out2;
	else
		color = out3;
}
