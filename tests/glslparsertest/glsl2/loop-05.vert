// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - loop body does not start a new scope */

void main()
{
  while (bool i = true)
    float i = gl_Vertex.x;

  gl_Position = gl_Vertex;
}
