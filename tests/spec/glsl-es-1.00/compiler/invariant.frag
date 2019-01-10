// [config]
// expect_result: pass
// glsl_version: 1.00
// [end config]

/* From Section 4.6.1 ("The Invariant Qualifier") GLSL ES 1.00 spec:
 *
 *     "Only the following variables may be declared as invariant:
 *      Built-in special variables output from the fragment shader."
 */

#version 100

invariant gl_FragColor;
invariant gl_FragData;

void main() { }
