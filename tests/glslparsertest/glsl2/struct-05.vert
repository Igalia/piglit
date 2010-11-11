// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - structure name conflicts with variable name in same scope */

uniform vec4 foo;

struct foo {
  float f;
  int i;
  bool b;
};

void main()
{
  foo foo;

  gl_Position = gl_Vertex;
}
