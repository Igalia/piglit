/* [config]
 * expect_result: fail
 * glsl_version: 1.50
 * [end config]
 */
#version 150

in float gl_ClipDistance[2];

void main()
{
    gl_Position = gl_Vertex;
}
