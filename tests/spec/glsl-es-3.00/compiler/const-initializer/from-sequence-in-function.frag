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

precision mediump float;

const float f = cos((1.0, 2.0));
out vec4 fragdata;

void main()
{
    fragdata = vec4(f);
}
