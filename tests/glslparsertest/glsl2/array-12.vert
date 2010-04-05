#version 120
/* PASS
 * The array is implicitly sized by the whole-array assignment.
 */

void main()
{
  vec4 a[];

  a = vec4 [2] (vec4(1.0), vec4(2.0));

  gl_Position = gl_Vertex;
}
