/* [config]
 * expect_result: fail
 * glsl_version: 1.30
 * [end config]
 */
#version 130

float gl_ClipDistance[2];

void main()
{
    gl_FragColor = vec4(0);
}
