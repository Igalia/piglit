// [config]
// expect_result: pass
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

#version 120
/* PASS - implicit conversions are done in array constructors */
void main()
{
  float a[];

  a = float[](4, 5);

  gl_Position = gl_Vertex;
}
