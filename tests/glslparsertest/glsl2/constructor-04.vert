// [config]
// expect_result: fail
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

#version 120
/* FAIL - matrix must be only parameter to matrix constructor */

uniform mat2 a;
uniform float x;

void main()
{
  mat2 b;

  b = mat2(a, x);

  gl_Position = gl_Vertex;
}
