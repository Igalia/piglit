// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

out vec4 out_color;

void main()
{
	/* x is neither an input nor output, so 'sample' is not
	 * legal here.
	 */
	sample vec4 x = vec4(1);
	out_color = x;
}

