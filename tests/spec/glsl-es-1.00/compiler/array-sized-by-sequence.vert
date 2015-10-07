#version 100

/* [config]
 * expect_result: fail
 * glsl_version: 1.00
 * [end config]
 *
 * The spec does not explicitly forbid this.  However, the normative grammar
 * in the specification does not allow this production.
 */

uniform float uf[1, 2];

void main()
{
    gl_Position = vec4(uf[0]);
}
