// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// From the MESA_shader_integer_functions spec:
// If <interpolant> is declared with the "noperspective"
// qualifier, the interpolated value will be computed without perspective
// correction.

#version 130
#extension GL_MESA_shader_integer_functions: require

noperspective in float v1;
noperspective in vec2 v2;
noperspective in vec3 v3;
noperspective in vec4 v4;

void main()
{
	vec4 res = vec4(0);

	res += vec4(interpolateAtCentroid(v1), 1, 1, 1);
	res += vec4(interpolateAtCentroid(v2), 1, 1);
	res += vec4(interpolateAtCentroid(v3), 1);
	res += interpolateAtCentroid(v4);

	gl_FragColor = res;
}
