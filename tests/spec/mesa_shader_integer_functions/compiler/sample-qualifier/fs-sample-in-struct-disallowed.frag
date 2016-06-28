// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

struct things {
	/* not allowed in struct declarations */
	sample vec4 x;
};
out vec4 out_color;

void main()
{
	out_color = vec4(1);
}

