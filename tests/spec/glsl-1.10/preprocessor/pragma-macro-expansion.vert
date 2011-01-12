/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 *
 * Page 11 (page 17 of the PDF) of the GLSL 1.10 spec says:
 *
 *     "Tokens following #pragma are not subject to preprocessor macro
 *     expansion."
 *
 * Therefore, debug(FOO) will be recognized as is.  Since debug can only take
 * the parameters "on" or "off", this will generate an error.
 */
#define FOO on
#pragma debug(FOO)

/* Some compilers generate spurious errors if a shader does not contain
 * any code or declarations.
 */
int foo(void) { return 1; }
