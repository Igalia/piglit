// [config]
// expect_result: pass
// glsl_version: 1.40
// require_extensions: GL_ARB_enhanced_layouts
// [end config]

#version 140
#extension GL_ARB_enhanced_layouts: require

#if !defined GL_ARB_enhanced_layouts
#  error GL_ARB_enhanced_layouts is not defined
#elif GL_ARB_enhanced_layouts != 1
#  error GL_ARB_enhanced_layouts is not equal to 1
#endif

/* Some compilers generate spurious errors if a shader does not contain
 * any code or declarations.
 */
int foo(void) { return 1; }
