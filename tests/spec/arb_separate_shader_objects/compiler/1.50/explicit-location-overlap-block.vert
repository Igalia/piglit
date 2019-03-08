// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_separate_shader_objects
// check_link: true
// [end config]
//
// Test for explicit varying location overlap by interface blocks
#version 150
#extension GL_ARB_separate_shader_objects : require

in vec4 piglit_vertex;

layout(location = 0) out Block {
	vec4 out1;
	vec4 out2;
} b;

layout(location = 1) out vec4 out3;

void main()
{
	gl_Position = piglit_vertex;
	b.out1 = vec4(1.0, 0.0, 0.0, 1.0);
	b.out2 = vec4(1.0, 1.0, 0.0, 1.0);
	out3 = vec4(0.0);
}
