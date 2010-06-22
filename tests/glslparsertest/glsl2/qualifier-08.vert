/* PASS - centroid and invariant are not reserved words in GLSL 1.10 */
uniform vec2 centroid;
uniform vec2 invariant;

void main()
{
  gl_Position = vec4(centroid, invariant);
}