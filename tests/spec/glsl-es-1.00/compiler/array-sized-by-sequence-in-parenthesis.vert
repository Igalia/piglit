#version 100

/* [config]
 * expect_result: pass
 * glsl_version: 1.00
 * [end config]
 *
 * While the sequence operator is specifically disallowed as a constant
 * expression in GLSL ES 3.0 and later, it is allowed in GLSL ES 1.00.
 */

uniform float uf[(1, 2)];

void main()
{
    gl_Position = vec4(uf[0]);
}
