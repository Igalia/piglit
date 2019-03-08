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

in vec4 piglit_vertex;

layout(location = 0) out vec4 out1[2][2];
layout(location = 3) out vec4 out2;

void main()
{
	gl_Position = piglit_vertex;
	out1[0][0] = vec4(1.0, 0.0, 0.0, 1.0);
	out1[0][1] = vec4(0.0, 1.0, 0.0, 1.0);
	out1[1][0] = vec4(0.0, 0.0, 1.0, 1.0);
	out1[1][1] = vec4(1.0, 1.0, 1.0, 1.0);
	out2 = vec4(0.0);
}
