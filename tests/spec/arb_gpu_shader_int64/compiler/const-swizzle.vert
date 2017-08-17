#version 330
#extension GL_ARB_gpu_shader_int64: require

/* [config]
 * expect_result: pass
 * glsl_version: 3.30
 * require_extensions: GL_ARB_gpu_shader_int64
 * [end config]
*/

uint64_t f() {
  return u64vec2(1UL, 2UL).y;
}
