// [config]
// expect_result: fail
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

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
