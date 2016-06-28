// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// From the MESA_shader_integer_functions spec:
// "It is an error to use centroid out or sample out in a fragment shader"

#version 130
#extension GL_MESA_shader_integer_functions: require

sample out vec4 x;			/* not allowed */
out vec4 out_color;

void main()
{
	out_color = x;
}

