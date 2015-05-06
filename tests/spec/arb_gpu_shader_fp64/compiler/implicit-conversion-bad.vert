// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader_fp64
// [end config]
//
// Test double -> float implicit conversion doesn't happen
// this tests a bug in mesa

#version 150
#extension GL_ARB_gpu_shader_fp64 : enable

float _float = 0.0f;
vec2 _vec2 = vec2(0.0f);
vec3 _vec3 = vec3(0.0f);
vec4 _vec4 = vec4(0.0f);

double _double = 0.0lf;
dvec2 _dvec2 = dvec2(0.0lf);
dvec3 _dvec3 = dvec3(0.0lf);
dvec4 _dvec4 = dvec4(0.0lf);

void test() {

	/* int can be converted to double (and for vectors of same) */
	_float = _double;
	_vec2 = _dvec2;
	_vec3 = _dvec3;
	_vec4 = _dvec4;
}
