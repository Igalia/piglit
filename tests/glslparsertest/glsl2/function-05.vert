// [config]
// expect_result: fail
// glsl_version: 1.10
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

/* FAIL - type qualifiers mismatch between declaration and definition */

void foo(float x, float y, float z, out float l);

void foo(float x, float y, float z, inout float l)
{
  l = x + y + z;
}

void main()
{
  float x;
  foo(1.0, 1.0, 1.0, x);
  gl_Position = vec4(x);
}
