// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

#version 120

/* Section 4.1.10 (Implicit Conversions) of the GLSL 1.20 spec says:
 *
 *    There are no implicit array or structure conversions.
 */
uniform float a[] = int[](1, 2, 3, 4);

void main()
{
    gl_Position = vec4(a[0], a[1], a[2], a[3]);
}
