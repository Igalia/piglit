// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

#version 130
#extension GL_MESA_shader_integer_functions: require

in float v1;
in vec2 v2;
in vec3 v3;
in vec4 v4;

void main()
{
	vec4 res = vec4(0);

	res += vec4(interpolateAtSample(v1, 0), 1, 1, 1);
	res += vec4(interpolateAtSample(v2, 1), 1, 1);
	res += vec4(interpolateAtSample(v3, 2), 1);
	res += interpolateAtSample(v4, 3);

	gl_FragColor = res;
}
