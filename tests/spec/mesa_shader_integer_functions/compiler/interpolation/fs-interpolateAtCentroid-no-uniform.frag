// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

uniform vec4 v4;

void main()
{
	vec4 res = vec4(0);

	// <interpolant> must be a shader input.
	res += interpolateAtCentroid(v4);

	gl_FragColor = res;
}
