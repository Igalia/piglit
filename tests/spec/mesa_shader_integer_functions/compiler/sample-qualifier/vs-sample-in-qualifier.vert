// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

in vec4 y;
sample in vec4 x;	/* this is not allowed */

void main()
{
	gl_Position = y;
}
