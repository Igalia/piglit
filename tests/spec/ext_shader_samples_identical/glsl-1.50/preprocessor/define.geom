// [config]
// expect_result: pass
// glsl_version: 1.50
// require_extensions: GL_EXT_shader_samples_identical
// [end config]

#extension GL_EXT_shader_samples_identical: require

#if !defined GL_EXT_shader_samples_identical
#  error GL_EXT_shader_samples_identical is not defined
#elif GL_EXT_shader_samples_identical != 1
#  error GL_EXT_shader_samples_identical is not equal to 1
#endif

/* Some compilers generate spurious errors if a shader does not contain
 * any code or declarations.
 */
int foo(void) { return 1; }
