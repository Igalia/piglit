#version 120
/* FAIL - the "Implicit Conversion" rules are used, not the "Conversion and
 * Scalar Constructors" rules.  Thus, float->int is not done implicitly.
 */
void main()
{
  int a[];

  a = int[](4.1, 5.5);

  gl_Position = gl_Vertex;
}
