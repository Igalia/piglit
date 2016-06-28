// [config]
// expect_result: fail
// glsl_version: 1.30
// require_extensions: GL_MESA_shader_integer_functions
// [end config]

// The MESA_shader_integer_functions spec says:
//    "Component selection operators (e.g., ".xy") may not be used when
//    specifying <interpolant>."

#version 130
#extension GL_MESA_shader_integer_functions: require

in vec2 v2;
in vec3 v3;
in vec4 v4;

void main()
{
	vec4 res = vec4(0);

	res += vec4(interpolateAtCentroid(v2.xy), 1, 1);
	res += vec4(interpolateAtCentroid(v3.xyz), 1);
	res += interpolateAtCentroid(v4.xyzw);

	gl_FragColor = res;
}
