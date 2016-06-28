// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

in vec4 y;
sample out vec4 x;

void main()
{
	x = y;
	gl_Position = y;
}
