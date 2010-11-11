// [config]
// expect_result: pass
// glsl_version: 1.20
//
// # NOTE: Config section was auto-generated from file
// # NOTE: 'glslparser.tests' at git revision
// # NOTE: 6cc17ae70b70d150aa1751f8e28db7b2a9bd50f0
// [end config]

#version 120
/* PASS */

void main()
{
  vec4 a[] = vec4 [] (vec4(1.0), vec4(2.0));

  gl_Position = gl_Vertex;
}
