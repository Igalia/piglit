// [config]
// expect_result: fail
// glsl_version: 1.20
// [end config]

#version 120

/* vec4 to vec3 is not listed in table in section 4.1.10 (Implicit
 * Conversions) of the GLSL 1.20 spec.
 */
uniform vec3 a = vec4(1.0);

void main()
{
    gl_Position = vec4(a.xyz, 1.0);
}
