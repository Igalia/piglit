// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - array constructors forbidden in GLSL 1.10
 *
 * This can also generate an error because the 'vec4[]' style syntax is
 * illegal in GLSL 1.10.
 */
void main()
{
  vec4 a[2] = vec4 [2] (vec4(1.0), vec4(2.0));

  gl_Position = gl_Vertex;
}
