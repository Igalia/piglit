// [config]
// expect_result: pass
// glsl_version: 3.00
// [end config]

/* From Section 4.6.1 ("The Invariant Qualifier") GLSL ES 3.00 spec:
 *
 *     "Only variables output from a shader can be candidates for invariance."
 */

#version 300 es

invariant out highp vec4 test;

void main() { }
