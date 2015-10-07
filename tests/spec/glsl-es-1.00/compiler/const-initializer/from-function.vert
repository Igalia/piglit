#version 100

/* [config]
 * expect_result: pass
 * glsl_version: 1.00
 * [end config]
 *
 * Section 5.10 (Constant Expressions) of the GLSL ES 1.00.17 spec says:
 *
 *     "A constant expression is one of
 *
 *         ...
 *
 *         - a built-in function call whose arguments are all constant
 *           expressions, with the exception of the texture lookup functions."
 */

const float f = cos(0.0);

void main()
{
    gl_Position = vec4(f);
}
