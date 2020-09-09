#version 100

/* [config]
 * expect_result: pass
 * glsl_version: 1.00
 * [end config]
 *
 * The spec does not list these functions as builtins or reserved words,
 * we should be able to declare such functions.
 */

float trunc(float f) { return f; }
float round(float f) { return f; }
float roundEven(float f) { return f; }
float modf(float f) { return f; }

void main()
{
    gl_Position = vec4(0.0);
}

