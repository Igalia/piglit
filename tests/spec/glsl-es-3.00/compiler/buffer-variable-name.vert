/* [config]
 * expect_result: pass
 * glsl_version: 3.00 es
 * [end config]
 *
 * Make sure that 'buffer' is a valid identifier in ES 3.0. It becomes a
 * keyword in ES 3.1
 */
#version 300 es

in vec4 position;

void main()
{
  float buffer = 2.0;
  gl_Position = position * buffer;
}
