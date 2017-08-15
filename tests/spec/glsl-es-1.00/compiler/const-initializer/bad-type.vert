#version 100

/* [config]
 * expect_result: fail
 * glsl_version: 1.00
 * [end config]
*/

vec2 f() {
  const float x = 1;
  return vec2(x, 0.0);
}
