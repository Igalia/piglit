/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 */
#version 110

vec4 gl_TexCoord[2];

void main()
{
    gl_FragColor = vec4(0);
}
