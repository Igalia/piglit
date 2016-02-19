// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader_fp64
// [end config]
//
// Test double -> float implicit conversion doesn't happen

#version 150
#extension GL_ARB_gpu_shader_fp64 : enable

vec3 _vec3 = vec3(0.0f);

dvec3 _dvec3 = dvec3(0.0lf);

void test() {

    /* double cannot be converted to float (and for vectors of same) */
    _vec3 = _dvec3;
}
