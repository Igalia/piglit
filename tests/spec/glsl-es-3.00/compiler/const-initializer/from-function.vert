#version 300 es

/* [config]
 * expect_result: pass
 * glsl_version: 3.00
 * [end config]
 */

const float f = cos(0.0);

void main()
{
    gl_Position = vec4(f);
}
