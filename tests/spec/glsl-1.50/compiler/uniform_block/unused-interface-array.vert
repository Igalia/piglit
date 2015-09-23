/* [config]
 * expect_result: pass
 * glsl_version: 1.50
 * [end config]
 *
 * Tests that shader still compiles with an unused uniform block. A packed
 * layout means the implementation can eliminate the block entirely.
 */
#version 150

layout(packed) uniform ArrayBlock
{
  mat4 a;
} i[4];

void main()
{
  gl_Position = vec4(1.0);
}
