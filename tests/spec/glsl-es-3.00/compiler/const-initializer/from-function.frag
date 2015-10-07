#version 300 es

/* [config]
 * expect_result: pass
 * glsl_version: 3.00
 * [end config]
 */

precision mediump float;

const float f = cos(0.0);
out vec4 fragdata;

void main()
{
    fragdata = vec4(f);
}
