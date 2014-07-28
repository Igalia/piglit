// [config]
// expect_result: pass
// glsl_version: 1.20
//
// [end config]

/* PASS
 *
 * From page 38 (page 44 of the PDF) of the GLSL 1.20 spec:
 *
 *    "The second and third expressions can be any type, as long their types
 *    match, or there is a conversion in Section 4.1.10 "Implicit Conversions"
 *    that can be applied to one of the expressions to make their types
 *    match."
 */
#version 120

uniform bool selector;
uniform vec4 a[2];
uniform vec4 b[2];
uniform int idx;

void main()
{
  gl_Position = (selector ? a : b)[idx];
}
