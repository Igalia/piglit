#version 300 es

/* [config]
 * expect_result: fail
 * glsl_version: 3.00
 * [end config]
 *
 * Section 4.3.3 "Constant Expressions" of the OpenGL GLSL ES 3.00.4 spec
 * says:
 *
 *     "However, the sequence operator ( , ) and the assignment operators ( =,
 *     +=, ...)  are not included in the operators that can create a constant
 *     expression."
 */

const float f = 3.0 + (1.0, 2.0);

void main()
{
    gl_Position = vec4(f);
}
