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
 *
 * While the sequence operator is specifically disallowed as a constant
 * expression in GLSL ES 3.0 and later, it is allowed in GLSL ES 1.00.
 */

const float f = cos((1.0, 2.0));

void main()
{
    gl_Position = vec4(f);
}
