// [config]
// expect_result: fail
// glsl_version: 1.50
// require_extensions: GL_ARB_gpu_shader_fp64
// [end config]
//
// Test double -> float implicit conversion doesn't happen

#version 150
#extension GL_ARB_gpu_shader_fp64 : enable

vec2 _vec2 = vec2(0.0f);

dvec2 _dvec2 = dvec2(0.0lf);

void test() {

    /* double cannot be converted to float (and for vectors of same) */
    _vec2 = _dvec2;
}
