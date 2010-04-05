#version 120
/* FAIL - type mismatch in whole-array assignment
 * The first assignment implies that the array `a' must be at least vec4[3].
 */

void main()
{
  vec4 a[];

  a[2] = gl_Vertex;
  a = vec4 [2] (vec4(1.0), vec4(2.0));

  gl_Position = gl_Vertex;
}
