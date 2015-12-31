// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_shader_draw_parameters
// [end config]

#version 140
#extension GL_ARB_shader_draw_parameters: require

#if !defined GL_ARB_shader_draw_parameters
#  error GL_ARB_shader_draw_parameters is not defined
#elif GL_ARB_shader_draw_parameters != 1
#  error GL_ARB_shader_draw_parameters is not equal to 1
#endif

/* Some compilers generate spurious errors if a shader does not contain
 * any code or declarations.
 */
int foo(void) { return 1; }
