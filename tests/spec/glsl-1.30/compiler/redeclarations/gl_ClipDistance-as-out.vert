/* [config]
 * expect_result: pass
 * glsl_version: 1.30
 * [end config]
 */
#version 130

out float gl_ClipDistance[2];

void main()
{
    gl_Position = gl_Vertex;
}
