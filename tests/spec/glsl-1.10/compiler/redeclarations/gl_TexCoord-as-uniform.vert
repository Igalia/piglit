/* [config]
 * expect_result: fail
 * glsl_version: 1.10
 * [end config]
 */
#version 110

uniform vec4 gl_TexCoord[2];

void main()
{
    gl_Position = gl_Vertex;
}
