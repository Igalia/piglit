// [config]
// expect_result: pass
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* PASS */

uniform ivec2 a;
uniform ivec2 b;

void main()
{
  mat2 c;

  c = mat2(a, b);

  gl_Position = gl_Vertex;
}
