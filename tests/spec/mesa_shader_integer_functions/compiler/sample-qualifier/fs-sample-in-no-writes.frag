// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// From the MESA_shader_integer_functions spec:
// "Variables declared as ..., or sample in may
// not be written to during shader execution."

#version 130
#extension GL_MESA_shader_integer_functions: require

sample in vec4 x;
out vec4 out_color;

void main()
{
	x = vec4(0);	/* not allowed */
	out_color = vec4(1);
}

