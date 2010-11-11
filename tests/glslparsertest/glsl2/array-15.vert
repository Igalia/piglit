// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - array declared with a size is redeclared
 * The only case that where the spec seems to allow array redeclaration is when
 * the first declaration lacks a size.
 */

void main()
{
  vec4 a[10];

  gl_Position = gl_Vertex;

  vec4 a[10];
}
