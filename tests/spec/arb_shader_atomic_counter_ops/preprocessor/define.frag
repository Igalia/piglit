// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_shader_atomic_counter_ops
// [end config]

#version 140
#extension GL_ARB_shader_atomic_counter_ops: require

#if !defined GL_ARB_shader_atomic_counter_ops
#  error GL_ARB_shader_atomic_counter_ops is not defined
#elif GL_ARB_shader_atomic_counter_ops != 1
#  error GL_ARB_shader_atomic_counter_ops is not equal to 1
#endif

/* Some compilers generate spurious errors if a shader does not contain
 * any code or declarations.
 */
int foo(void) { return 1; }
