// [config]
// expect_result: fail
// glsl_version: 1.40
// require_extensions: GL_ARB_separate_shader_objects
// check_link: true
// [end config]
//
// From the ARB_separate_shader_objects spec v.25:
//
//   " A program will fail to link if any two non-vertex shader input
//     variables are assigned to the same location."
#version 140
#extension GL_ARB_separate_shader_objects : require

uniform int i;

layout(location = 0) in mat2x4 in1;
layout(location = 1) in vec4 in2;

out vec4 color;

void main()
{
	if (i == 0)
		color = in1[0];
	else if (i == 1)
		color = in1[1];
	else
		color = in2;
}
