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

in vec4 piglit_vertex;

layout(location = 0) out mat2x4 out1;
layout(location = 1) out vec4 out2;

void main()
{
	gl_Position = piglit_vertex;
	out1[0] = vec4(1.0, 0.0, 0.0, 1.0);
	out1[1] = vec4(1.0, 1.0, 0.0, 1.0);
	out2 = vec4(0.0);
}
