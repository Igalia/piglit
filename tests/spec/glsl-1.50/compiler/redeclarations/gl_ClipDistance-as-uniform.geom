/* [config]
 * expect_result: fail
 * glsl_version: 1.50
 * [end config]
 */
#version 150

layout(points) in;
layout(points, max_vertices = 1) out;

uniform float gl_ClipDistance[2];

void main()
{
    gl_Position = vec4(0);
    EmitVertex();
}
