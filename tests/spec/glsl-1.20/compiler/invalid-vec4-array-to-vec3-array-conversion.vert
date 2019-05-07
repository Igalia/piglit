// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

#version 120

/* Section 4.1.10 (Implicit Conversions) of the GLSL 1.20 spec says:
 *
 *    There are no implicit array or structure conversions.
 */
uniform vec3 a[] = vec4[](vec4(1.0), vec4(2.0));

void main()
{
    gl_Position = vec4(a[0].xyz, 1.0);
}
